<?php

require_once('inc/schedule_utils.inc');
require_once('inc/masterheat.inc');
require_once('inc/kiosks.inc');
require_once('inc/racing-state.inc');
require_once('inc/rounds.inc');

// Time limit for finding the "best" schedule for injection of a single racer.
// (Usually this bounds the reschedule operation overall, but if an acceptable
// schedule can't be found in the minimal number of heats, each additional heat
// required resets the time limit.  It may be true that it's always possible to
// find an acceptable schedule in the minimal number of heats, in which case the
// disclaimer is moot.
define("INJECTION_TIME_LIMIT", 10);  // in seconds

// The overall request has to complete in this time, no matter how many racers
// we attempt to inject.
define("OVERALL_TIME_LIMIT", 1 * 60);

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

class ScheduleAdjuster {
  private $roundid;
  private $nlanes;
  private $lane_mask;  // 1's mark masked lanes NOT in use
  private $nlanes_usable;
  private $n_runs_per_lane;
  private $now_running;  // classid, roundid, heat, now_racing

  // $race_chart is an array of heats, each heat being an array of $nlanes
  // elements populated with racerids.  If $race_chart[$h] is null, then the
  // corresponding heat has already been run and cannot be rescheduled.
  private $race_chart;
  private $nheats;
  private $nheats_unraced;
  private $nheats_new;

  // For heats that have already been run and might need to be renumbered due to
  // heats being added or deleted to the race_chart.  Each entry is the heat
  // number of the corresponding already-run heat, or null for a not-yet-run
  // heat.  Initially, $heat_numbers[H] = H + 1 (e.g., the 0th entry is for heat
  // 1), but that can change as the race_chart is updated.
  private $heat_numbers;

  // Set true if the race_chart was updated for departed racer(s), added
  // racer(s), or squeezed-out byes.
  private $chart_modified = false;
  // We can't prove that adjustment is always possible.  If we attempted a
  // change but couldn't complete it, race_chart may be modified but shouldn't
  // be written back to the database.
  private $adjustment_failed = false;
  
  private $has_byes = false;  // Are there any byes in the existing chart?

  // An array indexed by lane, each value being an array of heat indices with
  // byes for that lane.
  private $byes;

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
                 'heat_numbers' => $this->heat_numbers,
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

  public function chart_debug() {
    return array_map(function(&$ht) {
      if ($ht === null)
        return $ht;
      return implode(" ", array_map(function($r) { return ($r === null) ? "-" : $r; }, $ht));
    },
      $this->race_chart);
  }

  public function __construct($roundid, $trace = false) {
    $this->time0 = time();
    $this->roundid = $roundid;
    $this->nlanes = get_lane_count();
    $this->lane_mask = intval(read_raceinfo('unused-lane-mask', 0));
    $this->nlanes_usable = $this->nlanes - count_one_bits($this->lane_mask);
    $this->n_runs_per_lane = n_runs_per_lane($roundid);
    $this->now_running = get_running_round();
    $this->load_race_chart();

    $heats_raced = 0;
    for ($h = 0; $h < count($this->heat_numbers); ++$h) {
      if ($this->heat_numbers[$h] !== null) {
        $this->race_chart[$h] = null;
        ++$heats_raced;
      }
    }
    $this->nheats = count($this->race_chart);
    $this->nheats_unraced = $this->nheats - $heats_raced;
    if ($trace) {
      $this->trace = array( 'trace', $this->to_json() );
    }
    $this->times['constructed'] = time() - $this->time0;
  }

  public function updated_race_chart() {
    if (!$this->chart_modified || $this->adjustment_failed) {
      return null;
    }
    return $this->race_chart;
  }

  public function renumbering_map() {
    // In heat_numbers, for already-run heats, $h + 1 is what the heat number
    // should be, $heat_numbers[$h] is what it currently is.  We want the
    // inverted map, so the index is the heat number to be assigned, and the
    // value is the current heat number.  In constructing the inverted map, we
    // strip out any already-run heats that don't need renumbering.
    $value = array();

    for ($h = 0; $h < count($this->heat_numbers); ++$h) {

      if ($this->heat_numbers[$h] === null ||
          $this->heat_numbers[$h] == $h + 1) {
        continue;
      }
      $fromheat = $this->heat_numbers[$h];
      $toheat = $h + 1;
      while ($toheat >= count($value)) {
        $value[] = null;
      }
      $value[$toheat] = $fromheat;
      if ($this->trace) $this->trace[] = "Renumber heat $fromheat to be heat $toheat";
    }

    if ($this->trace) $this->trace[] = array('renumbering_map', $value);
    return $value;
  }

  private function load_race_chart() {
    global $db;
    $this->race_chart = array();
    $this->heat_numbers = array();
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
        $this->heat_numbers[] = null;
      }
      if ($row['has_result']) {
        $this->heat_numbers[$row['heat'] - 1] = $row['heat'];
      }
      $this->race_chart[$row['heat'] - 1][$row['lane'] - 1] = intval($row['racerid']);
    }

    $this->look_for_byes();

    return $completed_heat_indices;
  }

  private function look_for_byes() {
    $this->has_byes = false;
    $this->byes = array_fill(0, $this->nlanes, null);
    for ($lane = 0; $lane < $this->nlanes; ++$lane) {
      $this->byes[$lane] = array();
      if (($this->lane_mask & (1 << $lane)) == 0) {  // Active lane
        for ($h = 0; $h < $this->nheats; ++$h) {
          if ($this->race_chart[$h] !== null && $this->race_chart[$h][$lane] === null) {
            $this->byes[$lane][] = $h;
            $this->has_byes = true;
          }
        }
      }
    }
    return $this->has_byes;
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

  // Compute the mean and standard deviation for the population of potential
  // matchups.  That population has size nracers * (nracers - 1), as it's the
  // space of pairs of distinct racers.
  private function compute_matchup_stats(&$matchups) {
    // $matchups won't include entries for racer pairs that never meet in a single heat.
    // Those matchup values of 0 can be ignored for the mean.
    $nracers = count($matchups);
    if ($nracers <= 1) return array('nracers' => $nracers);
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

  // Remove (convert to byes) the appearance of any racer that's no longer in
  // the round's Roster.
  public function remove_departed() {
    global $db;
    $stmt = $db->prepare('SELECT DISTINCT racerid '.sql_for_departed(':roundid'));
    $stmt->execute(array(':roundid' => $this->roundid));
    $departed = $stmt->fetchAll(PDO::FETCH_COLUMN, 0);
    foreach ($departed as $gone) {
      $this->has_byes = true;
      foreach ($this->race_chart as &$heat) {
        if ($heat != null) {
          for ($lane = 0; $lane < $this->nlanes; ++$lane) {
            if ($heat[$lane] == $gone) {
              $heat[$lane] = null;
              $this->chart_modified = true;
              if ($this->trace) {
                $this->trace[] = "Removed departed racer $gone from 0-based lane $lane";
                $this->trace[] = $this->chart_debug();
              }
            }
          }
        }
      }
    }
    $this->look_for_byes();
  }

  public function squeeze_out_byes() {
    while ($this->look_for_byes()) {
      if ($this->trace) {
        $this->trace[] = "Squeezing out byes: ".json_encode($this->byes);
      }
      for ($lane = 0; $lane < $this->nlanes; ++$lane) {
        if (($this->lane_mask & (1 << $lane)) == 0 &&  // Active lane
            count($this->byes[$lane]) == 0) {
          // There are byes in the chart, but there aren't any for $lane, so
          // squeezing out heats by populating byes won't be possible.
          if ($this->trace) {
            $this->trace[] = "No byes available for lane $lane";
          }
          return;
        }
      }
      if (!$this->squeeze_out_one_heat_of_byes()) {
        // Couldn't do it, don't keep trying.  squeeze_out_one_heat_of_byes
        // won't have changed the race_chart if unsuccessful.
        if ($this->trace) $this->trace[] = "squeeze_out_byes giving up.";
        return;
      }
    }
  }

  private function squeeze_out_one_heat_of_byes() {
    for ($h = 0; $h < $this->nheats; ++$h) {
      if ($this->race_chart[$h] == null) {
        // Already raced
        continue;
      }

      // An array of heat indices.  Each racer in lane L of heat $h will fill a
      // bye in heat $relos[L].
      $relos = array_fill(0, $this->nlanes, null);
      $h_plausible = true;
      for ($lane = 0; $lane < $this->nlanes; ++$lane) {
        if (($this->lane_mask & (1 << $lane)) != 0) {  // Masked-out lane
          if ($this->trace) $this->trace[] = "Lane ".($lane + 1)." is masked out";
          $relos[$lane] = null;
        } else {
          $candidate = $this->race_chart[$h][$lane];
          $relos[$lane] = null;
          foreach ($this->byes[$lane] as $bh) {  // Try each heat with a bye for $lane
            // - The racer currently in race_chart[$h][$lane] can't appear in
            //   race_chart[$bh].
            // - TODO Scoring for matchups?
            if (in_array($candidate, $this->race_chart[$bh])) {
              if ($this->trace)
                $this->trace[] = "Considering dissolving heat $h: "
                               ."Can't put racer $candidate in lane "
                               .($lane + 1)." of heat $bh: ".json_encode($this->race_chart[$bh]);
              continue;
            }
            $relos[$lane] = $bh;

            // TODO There are other options, we're just seizing the first one.
            break;
          }
          if (!$relos[$lane]) {
            $h_plausible = false;
            break;
          }
        }
      }

      if ($h_plausible) {
        // We like $h for distribution across the $relo heats.
        if ($this->trace) {
          $this->trace[] = "Squeezing out heat $h: ".implode(" ", $this->race_chart[$h]);
        }
        for ($lane = 0; $lane < $this->nlanes; ++$lane) {
          if ($relos[$lane] != null) {
            $this->race_chart[$relos[$lane]][$lane] = $this->race_chart[$h][$lane];
          }
        }
        array_splice($this->race_chart, $h, 1);
        array_splice($this->heat_numbers, $h, 1);
        $this->chart_modified = true;
        --$this->nheats;
        --$this->nheats_unraced;
        if ($this->roundid == $this->now_running['roundid']) {
          // Turn off "now racing" if we're changing the currently-running heat.
          set_racing_state(false);
        }
        return true;
      }
    }
    // Couldn't find a viable $h heat
    return false;
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

  // Inject one racer with n_runs_per_lane into the scheduled round.
  public function inject_new_racer($new_racer_id) {
    for ($run = 0; $run < $this->n_runs_per_lane; ++$run) {
      if ($this->trace) {
        $this->trace[] = "Injecting for run $run of ".$this->n_runs_per_lane;
      }

      $this->add_minimal_new_heats();
      // After adding the minimal number of new heats that might accommodate the
      // new racer, this outer loop reluctantly adds additional heats as needed.
      // Worst case, we need to add a new heat for each each usable lane, in
      // order to race the new racer alone in each heat.
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
        $this->race_chart
          = $this->make_new_injection_race_chart($this->best_proposal['injection_heats'],
                                                 $this->best_proposal['displacement_heats'],
                                                 $new_racer_id);
        $this->chart_modified = true;
        $this->times["run-$run-racechart"] = time() - $this->time0;
      } else {
        // We've never seen a case where a racer couldn't be incorporated, but
        // we don't have a proof that it always works.  We set this flag to
        // indicate that the race_chart array may be in some indeterminate state
        // after adding extra, perhaps unpopulated, heats.
        $this->adjustment_failed = true;
        if ($this->trace) $this->trace[] = "Adjustment failed.";
      }
    }
  }

  // Calculates the smallest number of heats that might accommodate a new racer,
  // and adds that many empty heats to race_chart.
  private function add_minimal_new_heats() {
    $this->nheats_new = max($this->has_byes ? 0 : 1, $this->nlanes_usable - $this->nheats_unraced);
    if ($this->trace) {
      $this->trace[] = "Minimum $this->nheats_new new heats";
    }

    for ($h = 0; $h < $this->nheats_new; ++$h) {
      $this->race_chart[] = array_fill(0, $this->nlanes, null);
      ++$this->nheats;
    }
  }
  // Writes injection_heats and displacement_heats, and then
  // inject_new_racer_at_lane recurses through all the lanes that need to be
  // populated.
  private function inject_new_racer_once($new_racer_id) {
    $this->deadline = time() + intdiv(INJECTION_TIME_LIMIT, $this->n_runs_per_lane);
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
      if (($this->lane_mask & (1 << $new_racer_lane)) != 0) {  // Masked-out lane
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
          if ($this->race_chart[$dheat] !== null &&
              !in_array($displaced, $this->race_chart[$dheat], true)) {
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
    // $score is variance of matchups, always >= 0; we want the lowest we can get.
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
      // TODO There's probably minimum possible variance other than 0 for a
      // chart, which would allow an earlier exit.
      if ($score == 0) {  // No need to keep looking for better
        $this->keep_going = false;
      }
    }
    ++$this->nproposals;
    if (time() > $this->deadline) {
      $this->keep_going = false;
    }
  }

  private function make_new_injection_race_chart($injection_heats, $displacement_heats,
                                                 $new_racer_id) {
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

  // Injects the new racer into race_chart, and constructs a new heat of the
  // displaced racers.
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
  
  private function score_proposal($new_racer_id) {
    $replacements = $this->make_replacement_heats($new_racer_id);
    $matchups = $this->compute_matchups($replacements);
    $stats = $this->compute_matchup_stats($matchups);
    return array('stats' => $stats, // nracers, mean, variance
                 // 'replacements' => $replacements,
                 // 'matchups' => $this->summarize_matchups($matchups),
    );
  }
}

// We may have squeezed out some byes, which may have changed the heat number(s)
// of some already-run heats.  We need an renumbering plan for the already-raced heats.
function update_race_chart($roundid, &$race_chart, &$renumbering_map) {
  global $db;
  // Tear down the unraced heats before re-inserting
  $stmt = $db->prepare('DELETE FROM RaceChart'
                       .' WHERE roundid = :roundid'
                       .'  AND finishtime IS NULL'
                       .'  AND finishplace IS NULL');
  $stmt->execute(array(':roundid' => $roundid));

  // Some already-run heats may require renumbering.
  //
  $stmt = $db->prepare('UPDATE RaceChart SET heat = :toheat'
                       .' WHERE roundid = :roundid AND heat = :fromheat');
  // $renumbering_map is (1-based) indexed by what the heat number should be,
  // and values are what the current (1-based) heat number is.
  $more_renumbering = true;
  while ($more_renumbering) {
    $more_renumbering = false;
    for ($h = 1; $h < count($renumbering_map); ++$h) {
      if ($renumbering_map[$h] === null) {
        continue;
      } else if ($renumbering_map[$h] < count($renumbering_map) &&
                 $renumbering_map[$renumbering_map[$h]] !== null) {
        // We can't renumber this heat yet, because there's still a heat using
        // that number before it gets renumbered itself.  Since we know that
        // there can't be any renumbering cycles, eventually this number will
        // become available.
        $more_renumbering = true;
      } else {
        $stmt->execute(array(':roundid' => $roundid,
                             ':fromheat' => $renumbering_map[$h],
                             ':toheat' => $h));
        $renumbering_map[$h] = null;
      }
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
if (!have_permission(CONTROL_RACE_PERMISSION)) {
  json_not_authorized();
} else {
  $conflicting = read_raceinfo('reschedule_underway');
  // The time limit check is to handle the case where an attempt timed out (by
  // PHP or web server) and wasn't able to clear the marker.
  if ($conflicting !== false && time() - $conflicting < 2 * OVERALL_TIME_LIMIT) {
    json_failure('busy', 'Another schedule adjustment is already underway');
    return;  // Quits the include()
  }
  write_raceinfo('reschedule_underway', time());
  try {
    $t0 = time();

    // For simulating slow responses:
    // sleep(20);  // seconds
    
    $adj = new ScheduleAdjuster($roundid, $trace);

    $adj->remove_departed();

    $t1 = time();

    if ($racerid) {
      $adj->inject_new_racer($racerid);
    } else {
      // Insert all the racers for this round not already in the schedule.
      $racer_stmt = $db->prepare('SELECT racerid FROM Roster'
                                 .' WHERE roundid = :roundid'
                                 .' AND (SELECT passedinspection FROM RegistrationInfo'
                                 .'      WHERE racerid = Roster.racerid) = 1'
                                 .' AND NOT EXISTS(SELECT 1 FROM RaceChart'
                                 .'                WHERE racerid = Roster.racerid'
                                 .'                AND roundid = :roundid2)');
      $racer_stmt->execute(array(':roundid' => $roundid,
                                 ':roundid2' => $roundid));
      foreach ($racer_stmt as $racer_row) {
        if (time() - $t1 < OVERALL_TIME_LIMIT) {
          record_action(array('action' => 'schedule.*reschedule',
                              'racerid' => $racer_row['racerid']));
          $adj->inject_new_racer($racer_row['racerid']);
        }
      }
    }

    $t2 = time();

    $adj->squeeze_out_byes();

    if ($trace) {
      json_out('adjuster', $adj->to_json());
    }

    // May be false if there are no changes, or a rescheduling somehow failed.
    $new_chart = $adj->updated_race_chart();

    if ($new_chart !== null) {
      $renumbering_map = $adj->renumbering_map();
      $ok = true;
      json_out('chart', $adj->chart_debug());

      $t3 = $t2;
      if (!$dry_run) {
        update_race_chart($roundid, $new_chart, $renumbering_map);
        $t3 = time();
        assign_master_heats();
      }
      $t4 = time();
      json_out("timing", array('load' => $t1 - $t0,
                               'inject' => $t2 - $t1,
                               'update' => $t3 - $t2,
                               'master-heat' => $t4 - $t3,
                               'internal' => $adj->times));
      if ($ok) {
        json_success();
        mark_all_kiosks_for_reload();
        require('ajax/query.poll.coordinator.inc');
      }
    } else {
      json_failure('nochart', 'Unable to build new schedule');
    }
  } finally {
    delete_from_raceinfo('reschedule_underway');
  }
}
