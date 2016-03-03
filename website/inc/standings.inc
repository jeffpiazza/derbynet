<?php
require_once('inc/data.inc');

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

function sql_scoring_expression($finishtime = 'finishtime') {
  // Assumes we're in the context of a GROUP BY for one racer and one roundid.
  return read_raceinfo_boolean('drop-slowest') 
      ? '(SUM('.$finishtime.') - MAX('.$finishtime.'))/(COUNT(*) - 1)'
      : 'SUM('.$finishtime.')/COUNT(*)';
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

// Returns false if there are no grand final rounds, otherwise the roundid of the
// grand-final round with the highest round number.
function last_grand_final_roundid() {
  return read_single_value('SELECT R2.roundid AS roundid'
                           .' FROM (SELECT classid, MAX(round) AS max_round'
                           .'       FROM Rounds'
                           .'       WHERE NOT EXISTS('
                           .'            SELECT 1 FROM RegistrationInfo'
                           .'            WHERE RegistrationInfo.classid = Rounds.classid)'
                           .'       GROUP BY classid) AS R1'
                           .' INNER JOIN Rounds AS R2'
                           .' ON R1.classid = R2.classid'
                           .' AND R1.max_round = R2.round',
                           array());
}

// Returns the list of roundids from which the field's overall standings should
// be assessed.
function final_roundids() {
  $last_grand = last_grand_final_roundid();
  if ($last_grand) {
    return array($last_grand);
  }
  return final_native_roundids();
}

// Returns summary results for each racer within each race they participated in.
// Each summary element gives round information (roundid, classid), racer
// information (racerid, name, carnumber), and outcome information (number of
// heats the racer has run in the round so far, best time, worst time, scoring
// time), and a flag telling whether this round's result counts toward any final
// ranking for the full field.
//
// $final_roundids is an array of roundid values for "final" rounds, i.e., the
// highest-numbered round for each class.

// Returns the final standings for the whole field, according to the last round
// raced for each class.  Racers who were eliminated in prior rounds are not
// included.  Does not account for any Grand Finals rounds.
function final_standings() {
  global $db;
  
  $roundids_for_groups = final_native_roundids();
  $roundids_for_supergroup = last_grand_final_roundid();
  if ($roundids_for_supergroup === false) {
    $roundids_for_supergroup = $roundids_for_groups;
  } else {
    $roundids_for_supergroup = array($roundids_for_supergroup);
  }
  // It's invalid SQL to test for ' IN () ', so make sure there's at least one
  // (invalid) roundid.
  $roundids_for_groups[] = 0;
  $roundids_for_supergroup[] = 0;

  // We want this "overall" set of results, ordered by time score.  For any
  // other rounds (Rounds.roundid NOT IN (0,...)), we're going to need to place
  // the values in a separate map by roundid.
  // 

  $scoring_expr = sql_scoring_expression('finishtime');

  $stmt = $db->query('SELECT Rounds.roundid, Rounds.classid, RegistrationInfo.racerid,'
                     .' class, firstname, lastname, carnumber,'
                     .' Rounds.roundid IN ('.implode(',', $roundids_for_groups).') AS for_group,'
                     .' Rounds.roundid IN ('.implode(',', $roundids_for_supergroup).') AS for_supergroup,'
                     .' COUNT(*) AS base, '.$scoring_expr.' AS avg,'
                     .' MIN(finishtime) AS best, MAX(finishtime) AS worst'
                     .' FROM '.inner_join('RegistrationInfo', 'RaceChart',
                                          'RaceChart.racerid = RegistrationInfo.racerid',
                                          'Rounds',
                                          'RaceChart.roundid = Rounds.roundid',
                                          'Classes',
                                          'RegistrationInfo.classid = Classes.classid')
                     .' WHERE passedinspection = 1 AND exclude = 0'
                     .' AND finishtime IS NOT NULL'
                     // Of these, only the roundid and racerid are meaningful grouping keys; each of the other
                     // values is uniquely determined by the racerid.
                     .' GROUP BY Rounds.roundid, RegistrationInfo.racerid, RegistrationInfo.classid, '
                     .'  class, firstname, lastname, carnumber'
                     //.' HAVING COUNT(*) = '.$nlanes -- racers who finished all their heats
                     .' HAVING COUNT(*) > 1'
                     .' ORDER BY '.$scoring_expr);
  $result = array();
  foreach ($stmt as $row) {
    $result[] = array('racerid' => $row['racerid'],
                      'roundid' => $row['roundid'],
                      'for_supergroup' => $row['for_supergroup'],
                      'for_group' => $row['for_group'],
                      'classid' => $row['classid'],  // The racer's 'native'
                                                     // class, not the round's
                                                     // class (e.g. Grand
                                                     // Finals)
                      'class' => $row['class'],
                      'firstname' => $row['firstname'],
                      'lastname' => $row['lastname'],
                      'carnumber' => $row['carnumber'],
                      'base' => $row['base'],  // Number of heats run in round
                      'time' => $row['avg'],
                      'best' => $row['best'],
                      'worst' => $row['worst']);
  }

  return $result;
}

// Collect the top N racer IDs in each (native) class
function top_finishers_by_class($n_per_class) {
  $top_finishers = array();
  foreach (final_standings() as $row) {
    if ($row['for_group']) {
      $racerid = $row['racerid'];
      $classid = $row['classid'];
      if (!isset($top_finishers[$classid])) {
        $top_finishers[$classid] = array();
      }
      if (count($top_finishers[$classid]) < $n_per_class) {
        $top_finishers[$classid][] = $racerid;
      }
    }
  }
  return $top_finishers;
}

// Collect the top N racer ids according to overall standings (doesn't account
// for Grand Finals rounds)
function top_finishers_overall($n_trophies) {
  $results = array();
  foreach (final_standings() as $row) {
    if ($row['for_supergroup']) {
      $results[] = $row['racerid'];
      if (count($results) >= $n_trophies) {
        break;
      }
    }
  }

  return $results;
}

?>