<?php

require_once('inc/schedule_utils.inc');
require_once('inc/masterheat.inc');

$roundid = $_POST['roundid'];
$racerid = $_POST['racerid'];
$dry_run = $_POST['dry-run'];
$trace = $_POST['trace'];

function n_runs_per_lane($roundid) {
  global $db;

  // Pick a populated lane
  $lane = read_single_value('SELECT MIN(lane) FROM RaceChart WHERE roundid = :roundid',
                            array(':roundid' => $roundid));
  $nracers = read_single_value('SELECT COUNT(DISTINCT racerid) FROM RaceChart'
                               .' WHERE roundid = :roundid AND lane = :lane',
                               array(':roundid' => $roundid,
                                     ':lane' => $lane));
  $nheats = read_single_value('SELECT COUNT(*) FROM RaceChart'
                               .' WHERE roundid = :roundid AND lane = :lane',
                               array(':roundid' => $roundid,
                                     ':lane' => $lane));
  return intdiv($nheats, $nracers);
}

class LateArrival {
  private $roundid;
  private $nlanes;
  private $lane_mask;  // 1's mark masked lanes not in use
  private $nlanes_usable;
  private $n_runs_per_lane;

  // $race_chart is an array of heats, each heat being an array of $nlanes
  // elements populated with racerids.  If $race_chart[$h] is null, then the
  // corresponding heat has already been run and cannot be rescheduled.
  private $race_chart;
  private $nheats;
  private $nheats_unraced;
  private $nheats_new;
  private $has_byes = false;  // Are there any byes in the existing chart?

  // Deadline (as a time() value) to give up searching for an optimal solution.
  private $deadline;
  private $keep_going = true;

  private $best_proposal = null;  // array( 'score', 'injection_heats', 'displacement_heats' )
  private $nproposals;

  private $trace = null;
  public $times = array();
  private $time0;

  public function to_json() {
    return array('roundid' => $this->roundid,
                 'nlanes' => $this->nlanes,
                 'lane_mask' => $this->lane_mask,
                 'nlanes_usable' => $this->nlanes_usable,
                 'n_runs_per_lane' => $this->n_runs_per_lane,
                 'race_chart' => $this->race_chart,
                 'nheats' => $this->nheats,
                 'nheats_unraced' => $this->nheats_unraced,
                 'nheats_new' => $this->nheats_new,
                 'has_byes' => $this->has_byes,
                 'best_proposal' => $this->best_proposal,
                 'nproposals' => $this->nproposals,
                 'trace' => $this->trace,
                 'times' => $this->times,
    );
  }

  public function __construct($roundid, $trace = false) {
    $this->time0 = time();
    $this->roundid = $roundid;
    $this->nlanes = get_lane_count();
    $this->lane_mask = intval(read_raceinfo('unused-lane-mask', 0));
    $this->nlanes_usable = $this->nlanes - count_one_bits($this->lane_mask);
    $this->n_runs_per_lane = n_runs_per_lane($roundid);
    $completed_heat_indices = $this->load_race_chart();
    foreach ($completed_heat_indices as $h) {
      $this->race_chart[$h - 1] = null;
    }
    $this->nheats = count($this->race_chart);
    $this->nheats_unraced = $this->nheats - count($completed_heat_indices);
    if ($trace) {
      $this->trace = array( 'trace' );
    }
    $this->times['constructed'] = time() - $this->time0;
  }

  private function load_race_chart() {
    global $db;
    $this->race_chart = array();
    $completed_heat_indices = array();
    $stmt = $db->prepare('SELECT heat, lane, racerid,'
                         .' (finishtime IS NOT NULL'
                         .'      OR finishplace IS NOT NULL) AS has_result'
                         .' FROM RaceChart'
                         .' WHERE roundid = :roundid'
                         .' ORDER BY heat, lane');
    $stmt->execute(array(':roundid' => $this->roundid));
    foreach ($stmt as $row) {
      while ($row['heat'] > count($this->race_chart)) {
        $this->race_chart[] = array_fill(0, $this->nlanes, null);
      }
      if ($row['has_result'] && !in_array($row['heat'], $completed_heat_indices, true)) {
        $completed_heat_indices[] = $row['heat'];
      }
      $this->race_chart[$row['heat'] - 1][$row['lane'] - 1] = intval($row['racerid']);
    }
    foreach($this->race_chart as &$heat) {
      if ($this->has_byes) {
        break;
      }
      for ($lane = 0; $lane < $this->nlanes; ++$lane) {
        if (($this->lane_mask & (1 << $lane)) == 0 && $heat[$lane] === null) {
          $this->has_byes = true;
          break;
        }
      }
    }

    return $completed_heat_indices;
  }

  // An assoc of assocs to keep track of how many times each pair of racers
  // appear in the same heat (a "matchup" between those racers).  The smaller of
  // the two racerids is the key to the first assoc, and the larger racerid is
  // the key to the second.  Only computes entries for pairs that appear together.
  private function compute_matchups($replacements = false) {
    // If provided, $replacements maps a heat index to an alternate version of
    // that heat that should be used in place of the canonical one in
    // $this->race_chart.
    if ($replacements === false) {
      $replacements = array();
    }
    $nheats = count($this->race_chart);
    $matchups = array();
    for ($h = 0; $h < $nheats; ++$h) {
      $heat = &$this->race_chart[$h];
      if (isset($replacements[strval($h)])) {
        $heat = &$replacements[strval($h)];
      }

      foreach ($heat as $racer1) {
        if ($racer1 === null) {
          continue;
        }
        if (!isset($matchups[$racer1])) {
          $matchups[$racer1] = array();
        }
        foreach ($heat as $racer2) {
          if ($racer2 === null || intval($racer1) >= intval($racer2)) {
            continue;
          }
          if (!isset($matchups[$racer1][$racer2])) {
            $matchups[$racer1][$racer2] = 0;
          }
          ++$matchups[$racer1][$racer2];
        }
      }
    }
    return $matchups;
  }

  // Compute the mean and standard deviation for the population of potential matchups.
  // That population has size nracers * (nracers - 1), as it's the space of pairs of distinct racers.
  private function compute_matchup_stats(&$matchups) {
    // $matchups won't include entries for racer pairs that never meet in a single heat.
    // Those matchup values of 0 can be ignored for the mean.
    $nracers = count($matchups);
    if ($nracers <= 1) return array('nracers' => $nracers);  // TODO
    $mean = 0;
    foreach ($matchups as $racer1 => &$pairings) {
      foreach ($pairings as $racer2 => $m) {
        // The $matchups structure only lists $racer1 < $racer2 cases, so we
        // double to account for $racer2 < $racer1 duals.
        $mean += 2 * $m;
      }
    }
    $mean = $mean / ($nracers * ($nracers - 1));
    unset($pairings);

    // Standard deviation:
    // sigma^2 = sum( (mean - m)^2 ) / (nracers * (nracers - 1))
    //         = sum( mean^2 -2*m*mean + m^2 ) / (nracers * (nracers - 1))
    //         = mean^2 - [ sum( 2 * m * mean - m^2 ) / (nracers * (nracers - 1)) ]
    // Here again, we can ignore the m == 0 values not explicitly represented in the matchups structure,
    // while double-counting the non-zero m's to account for reversed racer pairs.

    $sigma2 = 0;
    foreach ($matchups as $racer1 => &$pairings) {
      foreach ($pairings as $racer2 => $m) {
        $sigma2 -= 2 * (2 * $m * $mean - $m * $m);
      }
    }
    $sigma2 = $mean * $mean + $sigma2 / ($nracers * ($nracers - 1));
    
    return array('nracers' => $nracers,
                 'mean' => $mean,
                 'variance' => $sigma2);
  }

  private function summarize_matchups(&$matchups) {
    $summary = array();
    foreach ($matchups as $racer1 => &$pairings) {
      foreach ($pairings as $racer2 => $m) {
        $summary[] = array(intval($racer1), intval($racer2), $m);
      }
    }
    return $summary;
  }

  // An array of heat indicies.  For each lane L, $this->injection_heats[L]
  // tells the heat where the new racer might be injected.
  private $injection_heats;
  // If the race chart already has a racer in the proposed injection slot for
  // lane L, the that racer gets "displaced" to the same lane in one of the
  // newly added heats.  While there's usually only one added heat, in some
  // cases there may be more, so it's necessary to keep track of which of the
  // new heats is proposed for the displaced racer.
  private $displacement_heats;
  
  public function inject_new_racer($new_racer_id) {
    for ($run = 0; $run < $this->n_runs_per_lane; ++$run) {
      if ($this->trace) {
        $this->trace[] = "Injecting for run $run";
      }

      $this->add_minimal_new_heats();
      for ($extra = 0; $extra < $this->nlanes_usable; ++$extra) {
        $this->times["run-$run-extra-$extra-start"] = time() - $this->time0;
        $this->best_proposal = null;
        $this->nproposals = 0;
        $this->inject_new_racer_once($new_racer_id);
        if ($this->best_proposal !== null || !$this->keep_going) {
          if (!$this->keep_going) {
            $this->times["run-$run-timed-out"] = true;
          }
          $this->times["run-$run-injected"] = time() - $this->time0;
          // If we have a solution, or we failed because we ran out of time,
          // then we're done.
          break;
        }

        // Try again with an extra heat added
        if ($this->trace) {
          $this->trace[] = "* Trying again after $extra extra heat(s)";
        }
        $this->race_chart[] = array_fill(0, $this->nlanes, null);
        ++$this->nheats;
        ++$this->nheats_new;
      }

      if ($this->best_proposal !== null) {
        $this->race_chart = $this->make_new_race_chart($this->best_proposal['injection_heats'],
                                                       $this->best_proposal['displacement_heats'],
                                                       $new_racer_id);
        $this->times["run-$run-racechart"] = time() - $this->time0;
      } else {
        return null;
      }
    }
    return $this->race_chart;
  }
  private function add_minimal_new_heats() {
    if ($this->has_byes) {
      $this->nheats_new = max(0, $this->nlanes_usable - $this->nheats_unraced);
    } else {
      $this->nheats_new = max(1, $this->nlanes_usable - $this->nheats_unraced);
    }
    if ($this->trace) {
      $this->trace[] = "Minimum $this->nheats_new new heats";
    }

    for ($h = 0; $h < $this->nheats_new; ++$h) {
      $this->race_chart[] = array_fill(0, $this->nlanes, null);
      ++$this->nheats;
    }
  }
  private function inject_new_racer_once($new_racer_id) {
    $this->deadline = time() + intdiv(10, $this->n_runs_per_lane);  // 10-second overall deadline
    $this->keep_going = true;
    $this->best_proposal = null;

    $this->injection_heats = array_fill(0, $this->nlanes, null);
    $this->displacement_heats = array_fill(0, $this->nlanes, null);
    $this->inject_new_racer_at_lane($new_racer_id, /*$new_racer_lane*/0);
  }
  private function inject_new_racer_at_lane($new_racer_id, $new_racer_lane) {
    if ($this->trace) {
      $this->trace[] = "inject_new_racer_at_lane $new_racer_lane: "
                     .implode(" ", array_filter($this->injection_heats));
    }
    if (time() > $this->deadline) {
      $this->keep_going = false;
      return;
    }
    if ($new_racer_lane >= $this->nlanes) {
      if ($this->trace) {
        $this->trace[] = "Proposal! ".implode(" ", array_filter($this->injection_heats));
      }
      $this->consider_proposal($new_racer_id);
      if (!$this->keep_going) {
        return;
      }
    } else {
      if (($this->lane_mask & (1 << $new_racer_lane)) != 0) {
        $this->injection_heats[$new_racer_lane] = null;
        $this->displacement_heats[$new_racer_lane] = null;
        if ($this->trace) {
          $this->trace[] = "decline masked lane $new_racer_lane";
        }
        $this->inject_new_racer_at_lane($new_racer_id, 1 + $new_racer_lane);
      } else {
        for ($heat = 0; $heat < $this->nheats; ++$heat) {
          if (!$this->keep_going) {
            return;
          }
          $this->inject_new_racer_at_lane_in_heat($new_racer_id, $new_racer_lane, $heat);
        }
      }
    }
  }
  private function inject_new_racer_at_lane_in_heat($new_racer_id, $new_racer_lane, $heat) {
    if (time() > $this->deadline) {
      $this->keep_going = false;
      return;
    }
    // - Can't propose an already-completed heat.
    // - Can't plug the new racer into the same heat twice (from a previous run)
    // - Can't choose the same heat twice in the current proposal (which would put
    //   the new racer in two lanes at once)
    //
    // When searching $this->injection_heats, we really only want to search the
    // populated part, i.e., from 0 to $heat - 1.  That's why we clean up
    // $this->injection_heats.
    if ($this->race_chart[$heat] === null ||
        in_array($new_racer_id, $this->race_chart[$heat], true) ||
        in_array($heat, $this->injection_heats, true)) {
      if ($this->trace) {
        if ($this->race_chart[$heat] === null) {
          // - Can't propose an already-completed heat.
          $reason = "heat finished";
        } else if (in_array($new_racer_id, $this->race_chart[$heat], true)) {
          // - Can't plug the new racer into the same heat twice (from a previous run)
          $reason = "new racer already present";
        } else {
          // - Can't choose the same heat twice in the current proposal (which would put
          //   the new racer in two lanes at once)
          $reason = "new racer already injected";
        }
        $this->trace[] = "Reject heat $heat for lane $new_racer_lane because $reason: "
                       .implode(" ", array_filter($this->injection_heats));
      }
      // We can't use this heat in the proposal, so just move on to the next heat
    } else {
      $displaced = $this->race_chart[$heat][$new_racer_lane];
      $this->injection_heats[$new_racer_lane] = $heat;
      if ($displaced === null) {
        $this->inject_new_racer_at_lane($new_racer_id, 1 + $new_racer_lane);
      } else {
        for ($dheat = $this->nheats - $this->nheats_new; $dheat < $this->nheats; ++$dheat) {
          if (time() > $this->deadline) {
            $this->keep_going = false;
          }
          if (!$this->keep_going) {
            return;
          }
          if (!in_array($displaced, $this->race_chart[$dheat], true)) {
            if ($this->trace) {
              $this->trace[] = "Displace racer $displaced to heat $dheat for lane $new_racer_lane: "
                             .implode(" ", array_filter($this->injection_heats));
            }

            $this->displacement_heats[$new_racer_lane] = $dheat;
            $this->race_chart[$dheat][$new_racer_lane] = $displaced;

            $this->inject_new_racer_at_lane($new_racer_id, 1 + $new_racer_lane);

            $this->race_chart[$dheat][$new_racer_lane] = null;
            $this->displacement_heats[$new_racer_lane] = null;
          }
          else if ($this->trace) {
              $this->trace[] = "Can't displace racer $displaced to heat $dheat for lane $new_racer_lane: "
                             .implode(" ", array_filter($this->injection_heats));
          }
        }
      }
      $this->injection_heats[$new_racer_lane] = null;
    }
  }

  private function consider_proposal($new_racer_id) {
    $s0 = time();
    $score = $this->score_proposal($new_racer_id)['stats']['variance'];
    if (time() - $s0 > 1) {
      $this->times["scoring"] = time() - $s0;
    }
    if ($this->trace) {
      $this->trace[] = array('proposal-score' => $score);
    }
    if ($this->best_proposal === null || $this->best_proposal['score'] > $score) {
      $this->best_proposal = array('score' => $score,
                                   'injection_heats' => array_slice($this->injection_heats, 0),
                                   'displacement_heats' => array_slice($this->displacement_heats, 0));
      if ($this->trace) {
        $this->trace[] = 'selected';
      }
      if ($score == 0) {  // No need to keep looking for better
        $this->keep_going = false;
      }
    }
    ++$this->nproposals;
    if (time() > $this->deadline) {
      $this->keep_going = false;
    }
  }

  private function make_new_race_chart($injection_heats, $displacement_heats, $new_racer_id) {
    $chart = array();
    foreach ($this->race_chart as $heat) {
      $chart[] = $heat;
    }
    for ($lane = 0; $lane < $this->nlanes; ++$lane) {
      if ($injection_heats[$lane] === null) {
        continue;
      }
      if ($displacement_heats[$lane] !== null) {
        $chart[$displacement_heats[$lane]][$lane] = $chart[$injection_heats[$lane]][$lane];
      }
      $chart[$injection_heats[$lane]][$lane] = $new_racer_id;
    }
    return $chart;
  }

  private function make_replacement_heats($new_racer_id) {
    $replacements = array();
    // $this->displacement_heats have already been incorporated into the race
    // chart, so we only need to worry about the injection heats.
    for ($lane = 0; $lane < $this->nlanes; ++$lane) {
      $inj = $this->injection_heats[$lane];
      if ($inj === null) {
        continue;
      }
      $heat = $this->race_chart[$inj];
      $heat[$lane] = $new_racer_id;
      $replacements[strval($inj)] = $heat;
    }
    return $replacements;
  }
  
  function score_proposal($new_racer_id) {
    $replacements = $this->make_replacement_heats($new_racer_id);
    $matchups = $this->compute_matchups($replacements);
    $stats = $this->compute_matchup_stats($matchups);
    return array('stats' => $stats,
                 // 'replacements' => $replacements,
                 // 'matchups' => $this->summarize_matchups($matchups),
    );
  }
}

function update_race_chart($roundid, &$race_chart) {
  global $db;
  // TODO record_event
  // Tear down the unraced heats and re-insert
  $stmt = $db->prepare('DELETE FROM RaceChart WHERE roundid = :roundid AND heat = :heat');
  for ($heat = 0; $heat < count($race_chart); ++$heat) {
    if ($race_chart[$heat] !== null) {  // Already-raced heats are null
      $stmt->execute(array(':roundid' => $roundid,
                           ':heat' => $heat + 1));
    }
  }

  $classid = read_single_value('SELECT classid FROM Rounds WHERE roundid = :roundid',
                               array(':roundid' => $roundid));
  $stmt = $db->prepare('INSERT INTO RaceChart(roundid, heat, lane, racerid, classid)'
                       .' VALUES(:roundid, :heat, :lane, :racerid, :classid)');
  for ($h = 0; $h < count($race_chart); ++$h) {
    if ($race_chart[$h] !== null) {
      $heat = &$race_chart[$h];
      for ($lane = 0; $lane < count($heat); ++$lane) {
        if ($heat[$lane] !== null) {
          $stmt->execute(array(':roundid' => $roundid,
                               ':heat' => $h + 1,
                               ':lane' => $lane + 1,
                               ':racerid' => $heat[$lane],
                               ':classid' => $classid));
        }
      }
    }
  }
}

// =========================================================
if (have_permission(CONTROL_RACE_PERMISSION)) {

  $t0 = time();

  $late = new LateArrival($roundid, $trace);

  $t1 = time();

  $new_chart = $late->inject_new_racer($racerid);

  $t2 = time();

  if ($trace) {
    json_out('late_arrival', $late->to_json());
  }

  if ($new_chart !== null) {
    $ok = true;
    json_out('chart',
             array_map(function(&$ht) {
                         if ($ht === null)
                           return $ht;
                         return implode(" ", $ht); },
               $new_chart));
    $t3 = $t2;
    if (!$dry_run) {
      update_race_chart($roundid, $new_chart);
      $t3 = time();
      assign_master_heats();
    }
    $t4 = time();
    json_out("times", array('load' => $t1 - $t0,
                            'inject' => $t2 - $t1,
                            'update' => $t3 - $t2,
                            'master-heat' => $t4 - $t3,
                            'internal' => $late->times));
    if ($ok) {
      json_success();
    }
  } else {
    json_failure('nochart', 'Unable to build new schedule');
  }
           
} else {
  json_not_authorized();
}
