<?php
require_once('inc/data.inc');
require_once('inc/classes.inc');
require_once('inc/schema_version.inc');
require_once('inc/name-mangler.inc');
require_once('inc/ordinals.inc');
require_once('inc/aggregate_round.inc');

/////////////////////////////////////////////////////////////////////////////////////
// For a given roundid, it's straightforward to calculate standings within the
// round, provided all the heats have been run.  If not all heats have been run,
// compare each racer's scoring-expression if it's well-defined, otherwise treat
// as 9.999.  (Or, treat all missing heat times as 9.999...)
//
// Assuming roundid standings can be computed, define den (class) standing to be
// the standings of the highest-numbered round for that class.  This applies
// both to native classes, and to aggregate classes that have associated racing
// rounds.  If there are (say) 2 rounds of racing for the class, this means that
// racers eliminated in the first round don't having a class standing.
//
// Classes that don't have racing rounds (called "nonracing aggregate classes")
// are a little different.  Their standings are determined by the highest round
// of racing for each constituent class.  (If the constituent class is also a
// nonracing aggregate class, then its constituent classes are examined.)
//
// A native class can be partitioned into ranks (subgroups).  Standings for a
// rank are imposed by the standings for the class, with numbering considering
// only members of a given rank.
//
// For the whole field, define standings as:
//
//  - In the absence of any aggregate rounds, then take the highest-numbered
//    round from each (native) class, and combine into an aggregate standing.
//    Some classes may have more rounds than others.
//
//  - If there is a single aggregate class, then the round standings for the
//    highest-numbered aggregate round.  (That's a guess.)
//
//  - If there is more than one aggregate class, then there is no overall
//    standing.
/////////////////////////////////////////////////////////////////////////////////////

// First we construct a "result summary," which is an ordered array whose
// entries represent the performance of one racer in one racing round.  The
// result summary is ordered from best (least) score to worst.  The standings
// for any particular group, subgroup, aggregate group, supergroup, round, or
// subgroup-within-a-round are determined by the overall ordering.
//
// For each group, subgroup, aggregate group, supergroup, round, or
// subgroup-in-round, we can construct a "ladder," represented as an array of
// rungs, each rung being an array of racerids, the zero'th rung representing
// first place.  In the absence of ties, each rung holds exactly one racerid.
// If there's a tie, a rung will contain more than one racerid, and one or more
// subsequent rungs will be empty.
/////////////////////////////////////////////////////////////////////////////////////

function standings_key_superclass() { return 'superclass'; }
function standings_key_class($classid) { return 'c'.$classid; }
function standings_key_subgroup($rankid) { return 'r'.$rankid; }
function standings_key_round($roundid) { return 'round'.$roundid; }
function standings_key_round_subgroup($roundid, $rankid) { return 'rr'.$roundid.'-'.$rankid; }

function is_standings_key_class($key) { return $key[0] == 'c'; }
function is_standings_key_round_subgroup($key) { return $key[0] == 'r' && $key[1] == 'r'; }

function parse_standings_key_class($key) { return substr($key, 1); }
// list($roundid, $rankid)
function parse_standings_key_round_subgroup($key) { return explode('-', substr($key, 2)); }

// Assumes we're in the context of a GROUP BY for one racer and one roundid.
// $finishtime and $finishplace are, respectively, the sql expressions for
// accessing the finish time and finish place for the one race result.
//
// For place-based races, instead of scoring by (lowest) finish time, we score
// by finish place, exactly as if it were a time.  E.g., treat 1st place like it
// was 1.00 seconds, 2nd place as 2.00 seconds, and so on.
function sql_scoring_expression($finishtime, $finishplace) {
  global $dbtype;
  $finish = read_raceinfo_boolean('use-points') ? $finishplace.'*1.0' : $finishtime;
  return read_raceinfo_boolean('drop-slowest') 
      ? ($dbtype == 'sqlite'
         ?  "CASE COUNT(*)"
             ." WHEN 1 THEN MAX($finish)"
             ." ELSE (SUM($finish) - MAX($finish))/(COUNT(*) - 1)"
             ." END"
         : "(SUM($finish) - MAX($finish))/(COUNT(*) - 1)" )
      : "SUM($finish)/COUNT(*)";
}

// Returns the final roundids for all classes, either native classes or
// aggregate classes.
function final_roundids_for_all_classes() {
  global $db;
  $stmt = $db->query('SELECT R2.roundid AS roundid'
                     .' FROM (SELECT Rounds.classid, MAX(round) AS max_round'
                     .'       FROM Rounds'
                     .'       GROUP BY Rounds.classid) AS R1'
                     .' INNER JOIN Rounds AS R2'
                     .' ON R1.classid = R2.classid'
                     .' AND R1.max_round = R2.round');
  $roundids = array();
  foreach ($stmt as $row) {
	$roundids[] = $row['roundid'];
  }
  return $roundids;
}

// If there are no aggregate classes, returns false.
//
// If there's just one aggregate class, chooses the roundid of its highest
// round.  We're assuming that the single aggregate class is the Grand Finals
// round that determines overall standings.
//
// If there's more than one aggregate class, returns 0 (which means there's no
// round for determining overall standings).
function last_aggregate_roundid() {
  $agg_classes = aggregate_classes();
  if (count($agg_classes) == 0) {
    return false;
  }
  if (count($agg_classes) > 1) {
    // We decline to pick a roundid that establishes overall results
    return 0;
  }
  return read_single_value('SELECT R2.roundid AS roundid'
                           .' FROM (SELECT classid, MAX(round) AS max_round'
                           .'       FROM Rounds'
                           .'       WHERE classid = :classid'
                           .'       AND NOT EXISTS('
                           .'            SELECT 1 FROM RegistrationInfo'
                           .'            WHERE RegistrationInfo.classid = Rounds.classid)'
                           .'       GROUP BY classid) AS R1'
                           .' INNER JOIN Rounds AS R2'
                           .' ON R1.classid = R2.classid'
                           .' AND R1.max_round = R2.round',
                           array(':classid' => $agg_classes[0]));
}

// Returns true if we can assess standings for the whole field (supergroup) --
// either there are no aggregate classes or exactly one.
function has_supergroup_standings() {
  return last_aggregate_roundid() !== 0;
}

function supergroup_groups() {
  global $db;
  $agg_classes = aggregate_classes();
  if (count($agg_classes) == 0) {
    $classes = array();
    $stmt = $db->query('SELECT classid FROM Classes');
    foreach ($stmt as $row) {
      $classes[] = $row[0];
    }
    return $classes;
  } else if (count($agg_classes) == 1) {
    return array($agg_classes[0]['classid']);
  } else {
    return false;
  }
}


// Returns the inverted map taking constituent classids to the non-racing
// aggregate classes that depend on them.  If $with_supergroup, also includes
// entries for the supergroup, which is represented as a classid of -1.
function inverted_constituents($with_supergroup = false) {
  global $db;
  // Maps classids for aggregate classes to a list of classids of the constituents.
  $forward_map = array();
  $result = array();
  
  if ($with_supergroup) {
    $supergroup_groups = supergroup_groups();
    if ($supergroup_groups !== false) {
      $forward_map["-1"] = $supergroup_groups;
    }
  }

  if (schema_version() >= 3) {
    $stmt = $db->prepare('SELECT classid, constituents FROM Classes'
                         .' WHERE constituents != \'\''
                         .'  AND NOT EXISTS(SELECT 1 FROM Rounds'
                         .'                 WHERE Rounds.classid = Classes.classid)');
    $stmt->execute(array());
    foreach ($stmt as $row) {
      $constituents = explode(',', $row['constituents']);
      $forward_map[$row['classid']] = $constituents;
    }
  }

  foreach ($forward_map as $classid => $constituents) {
    foreach ($constituents as $c) {
      if (!isset($result[$c])) {
        $result[$c] = array();
      }
      $result[$c][] = $classid;
    }
  }

  // Transitive closure
  do {
    $repeat = false;
    foreach ($forward_map as $nr => $constituents) {
      if (isset($result[$nr])) {
        // Constituents of $nr are (get added as) constituents of the classes of
        // which $nr is a constituent
        // $aggregates are the aggregate classes that incorporate $nr as a constituent.
        $aggregates = $result[$nr];
        foreach ($constituents as $nr_constit) {
          foreach ($aggregates as $agg) {
            if (!in_array($agg, $result[$nr_constit])) {
              $result[$nr_constit][] = $agg;
              $repeat = true;
            }
          }
        }
      }
    }
  } while ($repeat);

  return $result;
}

// Returns a summary for each racer's result within each round they participated
// in.  The results are ordered best-to-worst across all the racers and rounds
// in the database.  Each summary element gives:
//    roundid -- round information
//    round_classid -- the classid associated with the round -- may be an
//        aggregate class.  For non-racing aggregate classes, the round entry is
//        repeated with the classid of the non-racing aggregate class, if this
//        round serves to establish standing in the class.
//    racerid, firstname, lastname, carnumber, carname, classid, rankid -- racer
//        information
//    classid, class -- name of racer's (native) class (i.e., not "Grand Finals")
//    rankid, rank -- name of racer's rank
//    base -- number of heats run in round
//    time -- "average" time in this round, possibly after discarding worst heat
//    best, worst -- fastest and slowest time in this round
function result_summary() {
  global $db;

  $scoring_expr = sql_scoring_expression('finishtime', 'finishplace');
  $raw_expr = read_raceinfo_boolean('use-points') ? 'finishplace' : 'finishtime';
  if (read_raceinfo_boolean('use-points')) {
    if (read_raceinfo_boolean('drop-slowest')) {
      $avg_expr = (1 + get_lane_count()).' * (COUNT(*) - 1) - (SUM(finishplace) - MAX(finishplace))';
    } else {
      $avg_expr = (1 + get_lane_count()).' * COUNT(*) - SUM(finishplace)';
    }
  } else {
    $avg_expr = $scoring_expr;
  }

  // GROUP BY makes each returned row a summary of the outcome for one racer in
  // one round.
  //
  // ORDER BY means rows are ordered from best showing to worst, which means
  // they're also ordered best to worst within any given group.
  $stmt = $db->query('SELECT Rounds.roundid,'
                     .' Rounds.classid as round_classid, '
                     .' RegistrationInfo.racerid, firstname, lastname, carnumber, carname,'
                     // classid is the racer's 'native' class, not the round's class (e.g. Grand Finals)
                     .' RegistrationInfo.classid, class, RegistrationInfo.rankid, rank,'
                     // base is the number of heats in the round
                     .' COUNT(*) AS base, '
                     .$scoring_expr.' AS score, '.$avg_expr.' AS avg,'
                     .' MIN('.$raw_expr.') AS best, MAX('.$raw_expr.') AS worst'
                     .' FROM '.inner_join('RegistrationInfo', 'RaceChart',
                                          'RaceChart.racerid = RegistrationInfo.racerid',
                                          'Rounds',
                                          'RaceChart.roundid = Rounds.roundid',
                                          'Classes',
                                          'RegistrationInfo.classid = Classes.classid',
                                          'Ranks',
                                          'RegistrationInfo.rankid = Ranks.rankid')
                     .' WHERE passedinspection = 1 AND exclude = 0'
                     .' AND (finishtime IS NOT NULL OR finishplace IS NOT NULL)'
                     // Of these, only the roundid and racerid are meaningful grouping keys; each of the other
                     // values is uniquely determined by the racerid.
                     .' GROUP BY Rounds.roundid, RegistrationInfo.racerid, RegistrationInfo.classid, '
                     .'  Rounds.classid, class, RegistrationInfo.rankid, rank, firstname, lastname, carnumber'
                     // lastname, firstname for reproducible ordering in the event of a tie
                     .' ORDER BY '.$scoring_expr.', lastname, firstname');
  $stmt->setFetchMode(PDO::FETCH_ASSOC);
  $result = array();
  foreach ($stmt as $row) {
    $result[] = $row;
  }

  return $result;
}

// Returns roundids for unscheduled or incompletely-raced rounds
function unfinished_rounds() {
  global $db;
  $stmt = $db->query('SELECT roundid, classid FROM Rounds'
                     .' WHERE NOT EXISTS(SELECT 1 FROM RaceChart'
                     .'                  WHERE RaceChart.roundid = Rounds.roundid)'
                     .' OR EXISTS(SELECT 1 FROM RaceChart'
                     .'           WHERE RaceChart.roundid = Rounds.roundid'
                     .'             AND finishtime IS NULL AND finishplace IS NULL)');
  $result = array();
  foreach ($stmt as $row) {
    $result[$row[0]] = $row[1];
  }
  return $result;
}

class StandingsTracker {
  private $last_score = -1;
  // The number of recorded scores for this tracker.  This lets us "skip ahead"
  // when there are ties.
  private $count = 0;

  // $result[$i] gives the racerids for ($i+1)'th place
  // E.g., [[11], [1], [21, 41], [], [31]] to indicate a tie for 3rd place.
  // (There are no 4th place racers in this example; racerid 31 is in 5th
  // place.)
  public $result = array();

  // Called in order by score, that is, $score is always no less than all
  // previous scores in all previous calls for this tracker.  Returns the
  // zero-based numeric place, with no indication for ties.
  public function AddEntry($score, $racerid) {
    ++$this->count;
    if ($score <= $this->last_score) {
      // A tie!
      $place = count($this->result) - 1;  // Index of last populated "place" in the results
      $this->result[$place][] = $racerid;
      // The tricky bit here is that only upon the arrival of the second racer
      // do we find out that there was a tie; we need to be able to go back to
      // the first racer and update.  That's why we make a second pass to mark
      // ties, rather than trying to do it on the fly here.
    } else {
      while (count($this->result) < $this->count - 1) {
        $this->result[] = array();  // Empty slots for ties
      }
      $this->result[] = array($racerid);
      $place = $this->count - 1;
    }

    $this->last_score = $score;
    return $place;
  }
}

// $finalized_only = true restricts the output to just those keys
// (class/rank/supergroup) for which final standings are available.  That is,
// standings that are yet to be settled, because there's more racing to do, will
// be omitted.
//
// compute_all_standings returns two values:
//
// list($ladders, $parallel) = compute_all_standings($result_summary);
//
// $ladders maps keys to a ladder of finishers for a category, described above.
//
// $parallel is an array of entries matching each of the entries in
// $result_summary.  Each entry in $parallel maps keys to a ladder index in the
// ladder for the same key.
function compute_all_standings($result_summary, $finalized_only = false) {
  $inverted_constituents = inverted_constituents(true);

  $final_rounds = final_roundids_for_all_classes();
  $unfinished_rounds = unfinished_rounds();
  $unfinished_classes = array();

  if ($finalized_only) {
    foreach ($unfinished_rounds as $roundid => $classid) {
      $unfinished_classes[] = $classid;
      if (isset($inverted_constituents[$classid])) {
        $unfinished_classes = array_merge($unfinished_classes,
                                          $inverted_constituents[$classid]);
      }
    }
  }

  $use_subgroups = read_raceinfo_boolean('use-subgroups');

  $trackers = array();  // An array of key => StandingsTracker

  $parallel = array();

  // $p is a reference to an element of $parallel.
  $f = function($key, $score, $racerid, &$p) use (&$trackers) {
    if (!isset($trackers[$key])) {
      $trackers[$key] = new StandingsTracker();
    }
    $place = $trackers[$key]->AddEntry($score, $racerid);
    $p[$key] = $place;
  };

  foreach ($result_summary as &$row) {
    $parallel[] = array();
    $p = &$parallel[count($parallel) - 1];

    $f('round'.$row['roundid'], $row['score'], $row['racerid'], $p);

    if (!isset($unfinished_round[$row['roundid']])) {
      if (in_array($row['roundid'], $final_rounds)) {
        $f(standings_key_class($row['round_classid']), $row['score'], $row['racerid'], $p);
        if ($use_subgroups) {
          $f(standings_key_subgroup($row['rankid']), $row['score'], $row['racerid'], $p);
        }

        if (isset($inverted_constituents[$row['round_classid']])) {
          foreach ($inverted_constituents[$row['round_classid']] as $agg_classid) {
            if (!in_array($agg_classid, $unfinished_classes)) {
              $key = $agg_classid < 0 ? 'supergroup' : standings_key_class($agg_classid);
              $place = $f($key, $row['score'], $row['racerid'], $p);
            }
          }
        }
      } else {
        if ($use_subgroups) {
          // For non-final rounds, we can compute a standing for the racer
          // within their subgroup within the current round only.
          $f(standings_key_round_subgroup($row['roundid'], $row['rankid']),
             $row['score'], $row['racerid'], $p);
        }
      }
    }
  }

  $results = array();
  foreach ($trackers as $key => &$tracker) {
    $results[$key] = $tracker->result;
  }
  foreach ($parallel as &$p) {
    foreach ($p as $key => &$place) {
      if (count($results[$key][$place]) > 1) {
        $place = 'T'.($place + 1);
      } else {
        $place = $place + 1;
      }
    }
  }
  return array($results, $parallel);
}

// If there's only one class, standings within that class are the same as
// standings within the supergroup, and there's no real point in showing them.
function show_group_standings() {
  return read_single_value('SELECT COUNT(*) FROM Classes') > 1;
}
function show_car_name_in_standings() {
  return read_single_value('SELECT COUNT(*) FROM RegistrationInfo'
                           .' WHERE carname IS NOT NULL AND carname > \'\'');
}

function write_standings_table_headers() {
  echo "<tr class='headers'>";
  echo "<th>Place</th>";
  echo "<th>Car Number</th>";
  echo "<th>Name</th>";
  if (show_car_name_in_standings()) {
    echo "<th>Car Name</th>";
  }
  if (show_group_standings()) {
    echo "<th class='group_column'>".group_label()."</th>";
    echo "<th class='ingroup_column'>In <span id='per-group-label'>".group_label()."</span></th>";
  }
  if (read_raceinfo_boolean('use-subgroups')) {
    echo "<th class='subgroup_column'>".subgroup_label()."</th>";
    echo "<th class='insubgroup_column'>In <span id='per-subgroup-label'>".subgroup_label()."</span></th>";
  }
  if (has_supergroup_standings()) {
    echo "<th class='insuper_column'>In ".supergroup_label()."</th>";
  }
  echo "<th class='inround_column'>In Round</th>";

  echo "<th>Heats</th>";

  if (read_raceinfo_boolean('use-points')) {
    echo "<th>Points (1st = ".get_lane_count().")</th>";
  } else {
    echo "<th>Average</th>";
  }
  echo "<th>Best</th>";
  echo "<th>Worst</th>";
  echo "</tr>\n";
}

function write_standings_table_rows(&$result_summary) {
  $name_style = read_raceinfo('name-style', FULL_NAME);
  $use_groups = show_group_standings();
  $use_subgroups = read_raceinfo_boolean('use-subgroups');
  $show_car_name = show_car_name_in_standings();
  $time_format = get_finishtime_formatting_string();

  list($finishers, $parallel) = compute_all_standings($result_summary);
  $i = 0;
  foreach ($result_summary as &$row) {
    $p = $parallel[$i];
    ++$i;

    $roundid = $row['roundid'];

    // js/standings-kiosk.js looks for data-first attribute for scrolling.
    echo "<tr data-roundid='".$roundid."' data-rankid='".$row['rankid']."'"
        ." data-for-supergroup='".isset($p['supergroup'])."'"
        .($i == 1 ? " data-first='1'" : "")
        .">";

    // First column:
    echo "<td>";
    {
      // echo json_encode($p);  // DEBUG

      echo "<div class='inround'>";
      {
        echo $p['round'.$roundid];
      }
      echo "</div>";

      echo "<div class='insuper'>";
      {
        if (isset($p['supergroup'])) {
          echo $p['supergroup'];
        }
      }
      echo "</div>";

      $classkey = standings_key_class($row['round_classid']);  // For round class
      foreach ($p as $key => $place) {
        if ($key != $classkey && is_standings_key_class($key)) {
          echo "<div class='nonracing-class hidden' data-standings-key='$key' data-agg-classid='".parse_standings_key_class($key)."'>";
          echo $p[$key];
          echo "</div>";
        }
      }

      if ($use_subgroups) {
        foreach ($p as $key => $place) {
          if (is_standings_key_round_subgroup($key)) {
            echo "<div class='hidden' data-standings-key='$key'>";
            echo $p[$key];
            echo "</div>";
          }
        }
      }
    }
    echo "</td>";

    echo "<td>".$row['carnumber']."</td>";
    echo "<td>".htmlspecialchars(mangled_name($row, $name_style), ENT_QUOTES, 'UTF-8')."</td>";
    if ($show_car_name) {
      echo "<td>".$row['carname']."</td>";
    }

    // Racer's (native) class and place in class/group
    if ($use_groups) {
      $key = 'c'.$row['classid'];  // Racer's classid
      echo "<td class='group_column'>".htmlspecialchars($row['class'], ENT_QUOTES, 'UTF-8')."</td>";
      echo "<td class='ingroup_column'>";
      if (isset($p[$key])) {
        echo $p[$key];
      }
      echo "</td>";
    }

    // Rank/subgroup and place in rank/subgroup
    if ($use_subgroups) {
      echo "<td class='subgroup_column'>".htmlspecialchars($row['rank'], ENT_QUOTES, 'UTF-8')."</td>";
      $key = standings_key_subgroup($row['rankid']);
      echo "<td class='insubgroup_column'>";
      if (isset($p[$key])) {
        echo $p[$key];
      }
      echo "</td>";
    }

    // Place in pack
    if (has_supergroup_standings()) {
      echo "<td class='insuper_column'>";
      if (isset($p['supergroup'])) {
        echo $p['supergroup'];
      }
      echo "</td>";
    }
    echo "<td class='inround_column'>".$p['round'.$roundid]."</td>";

    echo "<td>".$row['base']."</td>";

    if (read_raceinfo_boolean('use-points')) {
      echo "<td>".$row['avg']."</td>";
      echo "<td>".ordinal($row['best'])."</td>";
      echo "<td>".ordinal($row['worst'])."</td>";
    } else {
      echo "<td>".sprintf($time_format, $row['avg'])."</td>";
      echo "<td>".sprintf($time_format, $row['best'])."</td>";
      echo "<td>".sprintf($time_format, $row['worst'])."</td>";
    }
    echo "</tr>\n";
  }
}

// Returns an array of entries, each with a 'kind' and 'key' value, plus other
// values as follows:
//
// 'kind' = 'supergroup': key='supergroup', name
// 'kind' = 'class', key='c'.classid, name, roundid, classid;
// 'kind' = 'rank', key='r'.rankid, name, roundid, rankid
// 'kind' = 'round', key='round'.roundid, name, roundid
// 'kind' = 'agg-class', key='c'.classid, name, classid

function standings_catalog() {
  global $db;
  $use_subgroups = read_raceinfo_boolean('use-subgroups');

  $result = array();
  $stmt = $db->query('SELECT roundid, classid, round'
                     .' FROM Rounds'
                     .' ORDER BY round DESC');
  $rounds_by_class = array();
  foreach ($stmt as $row) {
    $classid = $row['classid'];
    if (!isset($rounds_by_class[$classid])) {
      $rounds_by_class[$classid] = array();
    }
    $rounds_by_class[$classid][] = array('roundid' => $row['roundid'],
                                         'round' => $row['round']);
  }

  $catalog = array();
  list($classes, $classseq, $ranks, $rankseq) = classes_and_ranks();

  if (has_supergroup_standings()) {
    $catalog[] = array('kind' => 'supergroup',
                       'key' => 'supergroup',
                       'name' => supergroup_label());
  }

  $rankseq_index = 0;
  foreach ($classseq as $classid) {
    $cl = $classes[$classid];
    if (isset($rounds_by_class[$classid])) {
      $rounds = $rounds_by_class[$classid];
      if (count($rounds) == 0) continue;  // this would be a bug
      // The final round for a class is listed first
      $final_round = array_shift($rounds);
      if (show_group_standings()) {
        $catalog[] = array('kind' => 'class',
                           'key' => standings_key_class($classid),
                           'name' => $cl['class'],
                           'classid' => $classid,
                           'roundid' => $final_round['roundid']);
      }
      if ($use_subgroups) {
        for (; $rankseq_index < count($rankseq); ++$rankseq_index) {
          $rank = $ranks[$rankseq[$rankseq_index]];
          if ($rank['classid'] != $classid) {
            break;
          }
          $catalog[] = array('kind' => 'rank',
                             'key' => standings_key_subgroup($rank['rankid']),
                             'name' => $cl['class'].' / '.$rank['rank'],
                             'roundid' => $final_round['roundid'],
                             'rankid' => $rankseq[$rankseq_index]);
        }
      }

      // Loop through the non-final rounds for the class
      foreach ($rounds as $round) {
        if (show_group_standings()) {
          $round_name =  $cl['class'].', Round '.$round['round'];
        } else {
          $round_name = 'Round '.$round['round'];
        }
        $catalog[] = array('kind' => 'round',
                           'key' => 'round'.$round['roundid'],
                           'name' => $round_name,
                           'roundid' => $round['roundid']);
        if ($use_subgroups) {
          foreach ($rankseq as $rankid) {
            $rank = $ranks[$rankid];
            if ($rank['classid'] == $classid) {
              $catalog[] = array('kind' => 'rank-round',
                                 'key' => standings_key_round_subgroup(
                                   $round['roundid'], $rank['rankid']),
                                 'name' => $round_name.' / '.$rank['rank'],
                                 'roundid' => $round['roundid'],
                                 'rankid' => $rankid);
            }
          }
        }
      }
    } else {
      // Nonracing aggregate class
      $catalog[] = array('kind' => 'agg-class',
                         'key' => standings_key_class($classid),
                         'name' => $cl['class'],
                         'classid' => $classid);
    }
  }
  return $catalog;
}
?>