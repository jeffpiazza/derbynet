<?php
require_once('inc/data.inc');
require_once('inc/schema_version.inc');
require_once('inc/name-mangler.inc');
require_once('inc/ordinals.inc');

// For a given roundid, it's straightforward to calculate standings within the
// round, provided all the heats have been run.  If not all heats have been run,
// compare each racer's scoring-expression if it's well-defined, otherwise treat
// as 9.999.  (Or, treat all missing heat times as 9.999...)
//
// Assuming roundid standings can be computed, define den (class) standing to be
// the standings of the highest-numbered round for that class.  If there are
// (say) 2 rounds of racing, this means that racers eliminated in the first
// round don't having a den standing.
//
// For the whole field, define standings as:
//
//  - If there are any Grand Final rounds, then the round standings for the
//    highest-numbered Grand Final round.  (Define Grand Final round.)
//
//  - In the absence of any Grand Final rounds, then take the highest-numbered
//    round from each class, and combine into an aggregate standing.  Some
//    classes may have more rounds than others.

// Assumes we're in the context of a GROUP BY for one racer and one roundid.
// $finishtime and $finishplace are, respectively, the sql expressions for
// accessing the finish time and finish place for the one race result.
//
// For place-based races, instead of scoring by (lowest) finish time, we score
// by finish place, exactly as if it were a time.  E.g., treat 1st place like it
// was 1.00 seconds, 2nd place as 2.00 seconds, and so on.
function sql_scoring_expression($finishtime, $finishplace) {
  $finish = read_raceinfo_boolean('use-points') ? $finishplace.'*1.0' : $finishtime;
  return read_raceinfo_boolean('drop-slowest') 
      ? '(SUM('.$finish.') - MAX('.$finish.'))/(COUNT(*) - 1)'
      : 'SUM('.$finish.')/COUNT(*)';
}

// Returns a list of roundids for the highest-numbered round for each native
// class with participants.
function final_native_roundids() {
  global $db;
  $stmt = $db->query('SELECT R2.roundid AS roundid'
                     .' FROM (SELECT Rounds.classid, MAX(round) AS max_round'
                     .'       FROM Rounds'
                     // The join with RegistrationInfo is to filter out the
                     // Grand Finals class, which has no registered members.
                     .'       INNER JOIN RegistrationInfo'
                     .'       ON Rounds.classid = RegistrationInfo.classid'
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

// Filters $roundids (an array of roundids) to just those rounds that have been
// completed (i.e., that have no uncompleted heats).
function completed_roundids($roundids) {
  global $db;
  if (count($roundids) == 0) {
    return $roundids;
  }
  $stmt = $db->query('SELECT roundid FROM Rounds'
                     .' WHERE roundid IN ('.implode(',', $roundids).')'
                     .' AND NOT EXISTS(SELECT 1 FROM RaceChart'
                     .'                WHERE RaceChart.roundid = Rounds.roundid'
                     .'                  AND finishtime IS NULL'
                     .'                  AND finishplace IS NULL)');
  $roundids = array();
  foreach ($stmt as $row) {
	$roundids[] = $row['roundid'];
  }
  return $roundids;
}

// Returns false if there are no grand final rounds, otherwise the roundid of the
// grand-final round with the highest round number.
//
// Note that we're identifying the grand final rounds in part by the class name,
// which has to match that from the roster.new action.
function last_grand_final_roundid() {
  return read_single_value('SELECT R2.roundid AS roundid'
                           .' FROM (SELECT classid, MAX(round) AS max_round'
                           .'       FROM Rounds'
                           .'       WHERE classid IN '
                           .'            (SELECT classid FROM Classes'
                           .'             WHERE class = \'Grand Finals\')'
                           .'       AND NOT EXISTS('
                           .'            SELECT 1 FROM RegistrationInfo'
                           .'            WHERE RegistrationInfo.classid = Rounds.classid)'
                           .'       GROUP BY classid) AS R1'
                           .' INNER JOIN Rounds AS R2'
                           .' ON R1.classid = R2.classid'
                           .' AND R1.max_round = R2.round',
                           array());
}

// Returns the list of roundids from which the field's overall standings should
// be assessed.  Returns an array of roundid values for "final" rounds, i.e.,
// the highest-numbered round for each class.
function final_roundids() {
  $last_grand = last_grand_final_roundid();
  if ($last_grand) {
    return array($last_grand);
  }
  return final_native_roundids();
}

// Returns summary results for each racer within each round they participated in.
// Each summary element gives round information (roundid, classid), racer
// information (racerid, name, carnumber), and outcome information (number of
// heats the racer has run in the round so far, best time, worst time, scoring
// time), and a flag telling whether this round's result counts toward any final
// ranking for the full field.
//
// Returns summary results for each racer within each race they participated in.
// Each summary element gives:
//    roundid, classid -- round information
//    racerid, firstname, lastname, carnumber, carname, classid, rankid -- racer information
//    class -- name of racer's (native) class (i.e., not "Grand Finals")
//    rank -- name of racer's rank
//    base -- number of heats run in round
//    time -- "average" time in this round, possibly after discarding worst heat
//    best, worst -- fastest and slowest time in this round
//    for_group -- 1 if this round determines group standings (i.e. is final round for group)
//    for_supergroup -- 1 if this entry determines full-field standing for this racer
function final_standings($completed_only = false) {
  global $db;
  
  $roundids_for_groups = final_native_roundids();
  $roundids_for_supergroup = last_grand_final_roundid();
  if ($roundids_for_supergroup === false) {
    $roundids_for_supergroup = $roundids_for_groups;
  } else {
    $roundids_for_supergroup = array($roundids_for_supergroup);
  }

  if ($completed_only) {
    $roundids_for_groups = completed_roundids($roundids_for_groups);
    // supergroup standings aren't known until ALL rounds have completed.
    if (read_single_value('SELECT COUNT(*) FROM RaceChart'
                          .' WHERE finishtime IS NULL AND finishplace IS NULL') > 0) {
      $roundids_for_supergroup = [];
    }
  }
  
  // It's invalid SQL to test for ' IN () ', so make sure there's at least one
  // (invalid) roundid.
  $roundids_for_groups[] = 0;
  $roundids_for_supergroup[] = 0;

  // We want this "overall" set of results, ordered by time score.  For any
  // other rounds (Rounds.roundid NOT IN (0,...)), we're going to need to place
  // the values in a separate map by roundid.
  // 

  $scoring_expr = sql_scoring_expression('finishtime', 'finishplace');
  $raw_expr = read_raceinfo_boolean('use-points') ? 'finishplace' : 'finishtime';
  // TODO Drop slowest for points racing?  This expression doesn't account for that, if it's meaningful.
  $avg_expr = read_raceinfo_boolean('use-points')
          ? (1 + get_lane_count()).' * COUNT(*) - SUM(finishplace)'
          : $scoring_expr;
  $stmt = $db->query('SELECT Rounds.roundid, Rounds.classid as round_classid, '
                     .' RegistrationInfo.racerid, firstname, lastname, carnumber, carname,'
                     // classid is the racer's 'native' class, not the round's class (e.g. Grand Finals)
                     .' RegistrationInfo.classid, class, RegistrationInfo.rankid, rank,'
                     .' Rounds.roundid IN ('.implode(',', $roundids_for_groups).') AS for_group,'
                     .' Rounds.roundid IN ('.implode(',', $roundids_for_supergroup).') AS for_supergroup,'
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
                     //.' HAVING COUNT(*) = '.$nlanes -- racers who finished all their heats
                     // lastname, firstname for reproducible ordering in the event of a tie
                     .' ORDER BY '.$scoring_expr.', lastname, firstname');
  $result = array();
  foreach ($stmt as $row) {
    $result[] = $row;
  }

  return $result;
}

function compute_standing(&$row, $key, &$all_so_far, &$result) {
  if (!isset($all_so_far[$key])) {
    $all_so_far[$key] = array('count' => 0,
                          'last_score' => -1,
                          'last_score_place' => 0,
                          'last_score_racerid' => 0);
  }

  $so_far = &$all_so_far[$key];

  ++$so_far['count'];
  if ($row['score'] <= $so_far['last_score']) {
    $result[$so_far['last_score_racerid']][$key] = 'T'.$so_far['last_score_place'];
    return 'T'.$so_far['last_score_place'];
  } else {
    $so_far['last_score'] = $row['score'];
    $so_far['last_score_place'] = $so_far['count'];
    $so_far['last_score_racerid'] = $row['racerid'];
    return $so_far['count'];
  }
}

// Returns map of racerid to
// { supergroup c(classid) r(rankid) (roundid) (roundid)... },
// where each element in the racerid's record gives the racer's place for the
// (sub/super)group or roundid.
function collect_standings(&$standings) {
  $result = array();
  // $all_so_far maps { supergroup c(classid) r(rankid) (roundid) ... }
  // to { count, last_score, last_score_place, last_score_racerid }.
  $all_so_far = array();
  foreach ($standings as &$row) {
    $racerid = $row['racerid'];
    $roundid = $row['roundid'];
    if (!isset($result[$racerid])) {
      $result[$racerid] = array();
    }
    $racer_result = &$result[$racerid];
    $racer_result[$roundid] = compute_standing($row, $roundid, $all_so_far, $result);
    if ($row['for_supergroup']) {
      $racer_result['supergroup'] = compute_standing($row, 'supergroup', $all_so_far, $result);
    }
    if ($row['for_group']) {
      $classid = $row['classid'];
      $rankid = $row['rankid'];
      $racer_result['c'.$classid] = compute_standing($row, 'c'.$classid, $all_so_far, $result);
      $racer_result['r'.$rankid] = compute_standing($row, 'r'.$rankid, $all_so_far, $result);
    }
  }
  return $result;
}

// Returns an array mapping place (first place is 0th element) to an array of
// racerids, the array having more than one element if there's a tie.
//
// E.g., returns [[11], [1], [21, 41], [], [31]] to indicate a tie for 3rd place.
//
// If $rankid is non-null, results are within that rank; if $classid is
// non-null, results are within that class; if both are null, results are within
// the supergroup.
function top_finishers($classid, $rankid, &$standings) {
  $result = array();
  $so_far = array('count' => 0,
                  'last_score' => -1);
  foreach ($standings as &$row) {
    $ok = true;
    if (isset($rankid)) {
      $ok = $ok && $row['rankid'] == $rankid;
    }
    if (isset($classid)) {
      $ok = $ok && $row['classid'] == $classid;
    }
    if (isset($classid) || isset($rankid)) {
      $ok = $ok && $row['for_group'];
    } else {
      $ok = $ok && $row['for_supergroup'];
    }
    if ($ok) {
      ++$so_far['count'];
      if ($row['score'] <= $so_far['last_score']) {
        // A tie!
        $result[count($result) - 1][] = $row['racerid'];
      } else {
        while (count($result) < $so_far['count'] - 1) {
          $result[] = array();  // Empty slots for ties
        }
        $result[] = array($row['racerid']);
        $so_far['last_score'] = $row['score'];
      }
    }
  }
  return $result;
}

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
    echo "<th>".group_label()."</th>";
    echo "<th class='ingroup'>In <span id='per-group-label'>".group_label()."</span></th>";
  }
  if (read_raceinfo_boolean('use-subgroups')) {
    echo "<th>".subgroup_label()."</th>";
    echo "<th class='insubgroup'>In <span id='per-subgroup-label'>".subgroup_label()."</span></th>";
  }
  echo "<th class='insuper'>In ".supergroup_label()."</th>";

  echo "<th>Heats</th>";

  if (read_raceinfo_boolean('use-points')) {
    echo "<th>Total Points (1st = ".get_lane_count().")</th>";
  } else {
    echo "<th>Average</th>";
  }
  echo "<th>Best</th>";
  echo "<th>Worst</th>";
  echo "</tr>\n";
}

function write_standings_table_rows(&$standings) {
  $standings_by_racer = collect_standings($standings);
  $name_style = read_raceinfo('name-style', FULL_NAME);
  $use_groups = show_group_standings();
  $use_subgroups = read_raceinfo_boolean('use-subgroups');
  $show_car_name = show_car_name_in_standings();
  $first = true;
  foreach ($standings as &$row) {
    $racerid = $row['racerid'];
    $roundid = $row['roundid'];
    $classid = $row['classid'];
    $rankid = $row['rankid'];
    $this_racer_standings = $standings_by_racer[$racerid];
    echo "<tr data-roundid='".$roundid."' data-rankid='".$row['rankid']."'"
        ." data-for-supergroup='".$row['for_supergroup']."'"
        .($first ? " data-first='1'" : "")
        .">";
    $first = false;

    // Cells in the first column contain two separate <div>s, only one of which is
    // made visible at any given time.  If we're displaying the standings for a
    // single round, the first column gives the place within the round.  If we're
    // displaying the standings within the supergroup ("All"), we display instead
    // the racer's place within the pack (same as the "In Pack" column).
    echo "<td>";
    echo "<div class='inround'>";
    {
      echo $this_racer_standings[$roundid];
    }
    echo "</div>";
    echo "<div class='insuper'>";
    {
      if ($row['for_supergroup'] && isset($this_racer_standings['supergroup']))
        echo $this_racer_standings['supergroup'];
    }
    echo "</div>";
    echo "</td>";
  
    echo "<td>".$row['carnumber']."</td>";
    echo "<td>".htmlspecialchars(mangled_name($row, $name_style), ENT_QUOTES, 'UTF-8')."</td>";
    if ($show_car_name) {
      echo "<td>".$row['carname']."</td>";
    }

    // Racer's (native) class and place in class/group
    if ($use_groups) {
      echo "<td>".htmlspecialchars($row['class'], ENT_QUOTES, 'UTF-8')."</td>";
      echo "<td class='ingroup'>";
      if ($row['for_group'] && isset($this_racer_standings['c'.$classid]))
        echo $this_racer_standings['c'.$classid];
      echo "</td>";
    }

    // Rank/subgroup and place in rank/subgroup
    if ($use_subgroups) {
      echo "<td>".htmlspecialchars($row['rank'], ENT_QUOTES, 'UTF-8')."</td>";

      echo "<td class='insubgroup'>";
      if ($row['for_group'] && isset($this_racer_standings['r'.$rankid])) {
        echo $this_racer_standings['r'.$rankid];
      }
      echo "</td>";
    }

    // Place in pack
    echo "<td class='insuper'>";
    if ($row['for_supergroup'] && isset($this_racer_standings['supergroup']))
      echo $this_racer_standings['supergroup'];
    echo "</td>";
  
    echo "<td>".$row['base']."</td>";

    if (read_raceinfo_boolean('use-points')) {
      echo "<td>".$row['avg']."</td>";
      echo "<td>".ordinal($row['best'])."</td>";
      echo "<td>".ordinal($row['worst'])."</td>";
    } else {
      echo "<td>".sprintf('%5.3f', $row['avg'])."</td>";
      echo "<td>".sprintf('%5.3f', $row['best'])."</td>";
      echo "<td>".sprintf('%5.3f', $row['worst'])."</td>";
    }
    echo "</tr>\n";
  }
}

// Returns an array of {roundid, name} entries for each racing round.
//
// The 'name' for final round is just the class name; for non-final rounds, a ",
// Round <n>" suffix is added.
function rounds_for_standings($roundid = false) {
  global $db;
  $result = array();
  $stmt = $db->query('SELECT class, round, R1.roundid, R1.classid,'
                     .' (SELECT MAX(round) FROM Rounds R2 WHERE R2.classid = R1.classid) AS max_round'
                     .' FROM Rounds R1'
                     .' INNER JOIN Classes'
                     .' ON R1.classid = Classes.classid'
                     .($roundid ? ' WHERE R1.roundid = '.$roundid : '')
                     .' ORDER BY '
                     .(schema_version() >= 2 ? 'Classes.sortorder, ' : '')
                     .'class, round DESC');
  foreach ($stmt as $row) {
    $t = $row['class'];
    if ($row['round'] < $row['max_round']) {
      $t .= ', Round '.$row['round'];
    }
    $result[] = array('roundid' => $row['roundid'],
                      'name' => $t);
  }

  foreach ($result as &$round) {
    $sort = (schema_version() >= 2 ? 'Ranks.sortorder, ' : '').'rank';
    $stmt = $db->prepare('SELECT DISTINCT Ranks.rankid, '.$sort
                         .' FROM '.inner_join('RaceChart', 'RegistrationInfo',
                                              'RaceChart.racerid = RegistrationInfo.racerid',
                                              'Ranks', 'RegistrationInfo.rankid = Ranks.rankid')
                         .' WHERE roundid = :roundid'
                         .' ORDER BY '.$sort);
    $stmt->execute(array(':roundid' => $round['roundid']));
    $round['ranks'] = array();
    foreach ($stmt as $rank) {
      $round['ranks'][] = array('rankid' => $rank['rankid'],
                                'name' => $rank['rank']);
    }
  }

  return $result;
}

?>