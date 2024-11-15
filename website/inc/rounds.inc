<?php
require_once('inc/schema_version.inc');
require_once('inc/aggregate_round.inc');

/*
Racing groups for different scheduling strategies:

Regular schedules:
  roundid
  round
  classid
  class
  groupid
  roundname = <Class>, Round <Round>

Master schedules:
  GroupID = Round
  GroupName = Round <Round>
 */

function find_roundid($classid, $roundno) {
  return read_single_value('SELECT roundid FROM Rounds'
                           .' WHERE classid = :classid'
                           .'  AND round = :roundno',
                           array(':classid' => $classid,
                                 ':roundno' => $roundno));
}

// Returns each distinct Rounds entry:
//  {roundid, round, classid, class, groupid(=roundid), roundname}
// By default, ordered primarily by class
// all_rounds(false) ordered primarily by round, then by class within each round.
function all_rounds($by_class = true) {
  global $db;
  $use_groups = use_groups();
  $stmt = $db->query('SELECT roundid, roundid AS groupid, round,'
                     .' Classes.classid, class,'
                     .($use_groups ? "class || ', ' || " : "")
                     .'\'Round \' || round AS roundname'
                     .' FROM Rounds INNER JOIN Classes'
                     .' ON Rounds.classid = Classes.classid'
                     .' ORDER BY '
                     .($by_class ? '' : 'round, ')
                     .(schema_version() >= 2 ? 'Classes.sortorder, ' : '')
                     .'class, round');
  return $stmt->fetchAll(PDO::FETCH_ASSOC);
}

function sql_for_unscheduled($roundid_expr) {
  return 'FROM Roster'
    .'    INNER JOIN RegistrationInfo'
    .'    ON Roster.racerid = RegistrationInfo.racerid'
    .'   WHERE Roster.roundid = '.$roundid_expr
    .'      AND RegistrationInfo.passedinspection <> 0'
    .'      AND NOT EXISTS(SELECT 1 FROM RaceChart'
    .'        WHERE RaceChart.roundid = Roster.roundid'
    .'          AND RaceChart.racerid = Roster.racerid)';
}
function sql_for_departed($roundid_expr) {
  return 'FROM RaceChart'
    .'         WHERE roundid = '.$roundid_expr
    .'         AND (finishtime IS NULL'
    .'          AND finishplace IS NULL)'
    .'         AND NOT EXISTS(SELECT 1 FROM Roster'
    .'                                 JOIN RegistrationInfo'
    .'                                 USING (racerid)'
    .'                        WHERE passedinspection <> 0'
    .'                          AND racerid = RaceChart.racerid'
    .'                          AND roundid = RaceChart.roundid)';
}

// Each entry has:
//  {roundid, round, classid, class, groupid(=roundid), roundname,
//    aggregate(0 or 1),
//    roster_size -- number of distinct racers in the round roster,
//    passed -- number of racers in the round roster who've passed inspection
//    unscheduled --  number of passed racers in the round roster without RaceChart entries
//    departed -- number of racers in yet-to-run heats who are no longer in the roster
//    heats_scheduled -- number of heats scheduled in the round
//    heats_run -- number of completed heats in the round
function all_rounds_with_counts($by_class = true) {
  global $db;
  $use_groups = use_groups();
  $stmt = $db->query('SELECT roundid, roundid AS groupid, round,'
                     .' Classes.classid, class,'
                     .(schema_version() >= 3 ? ' constituents > \'\' AS aggregate,' : '0 AS aggregate,')
                     .($use_groups ? " class || ', ' ||" : "")
                     .' \'Round \' || round AS roundname,'
                     .' (SELECT COUNT(*) FROM Roster'
                     .'   WHERE Roster.roundid = Rounds.roundid) AS roster_size,'
                     .' (SELECT COUNT(*) FROM Roster'
                     .'    INNER JOIN RegistrationInfo'
                     .'    ON Roster.racerid = RegistrationInfo.racerid'
                     .'   WHERE Roster.roundid = Rounds.roundid'
                     .'   AND RegistrationInfo.passedinspection <> 0) AS passed,'
                     .' (SELECT COUNT(*) '.sql_for_unscheduled('Rounds.roundid')
                                          .') AS unscheduled,'
                     .' (SELECT COUNT(DISTINCT racerid) '.sql_for_departed('Rounds.roundid')
                                                         .') AS departed,'
                     .' (SELECT COUNT(DISTINCT heat) FROM RaceChart'
                     .'         WHERE roundid = Rounds.roundid) AS heats_scheduled,'
                     .' (SELECT COUNT(DISTINCT heat) FROM RaceChart'
                     .'         WHERE roundid = Rounds.roundid'
                     .'         AND (finishtime IS NOT NULL'
                     .'           OR finishplace IS NOT NULL)) AS heats_run'
                     .' FROM Rounds INNER JOIN Classes'
                     .' ON Rounds.classid = Classes.classid'
                     .' ORDER BY '
                     .($by_class ? '' : 'round, ')
                     .(schema_version() >= 2 ? 'Classes.sortorder, ' : '')
                     .'class, round');
  return $stmt->fetchAll(PDO::FETCH_ASSOC);
}

// Returns map of $classid => $roundid to get the highest-numbered round for each class.
function class_highest_rounds() {
  global $db;
  $stmt = $db->query('SELECT classid, roundid FROM Rounds r1'
                     .' WHERE round = (SELECT MAX(round) FROM Rounds r2'
                     .'                WHERE r2.classid = r1.classid)');
  $stmt->execute();
  $round_by_class = array();
  foreach ($stmt as $row) {
    $round_by_class[$row['classid']] = $row['roundid'];
  }
  return $round_by_class;
}

// Returns array of roundids for highest rounds for passed-in classids.
function highest_rounds($classids_str) {
  global $db;
  $stmt = $db->prepare('SELECT roundid FROM Rounds r'
                       .' WHERE classid IN ('.$classids_str.')'
                       .'  AND round = (SELECT MAX(round) FROM Rounds r2'
                       .'               WHERE r2.classid = r.classid)');
  $stmt->execute(array());
  return $stmt->fetchAll(PDO::FETCH_COLUMN, 0);
}

// Returns each distinct Rounds entry, except if master scheduling, in which
// case, one entry per ordinal round (likely just one, or possibly 1st and 2nd).
function all_schedule_groups() {
  global $db;
  if (use_master_sched()) {
    $groups = array();
	foreach ($db->query('SELECT DISTINCT round'
						.' FROM Rounds'
						.' ORDER BY round') as $row) {
      $groups[] = array('round' => $row['round'],
                        'groupid' => $row['round'],
						'roundname' => 'Round '.$row['round']);
    }

    return $groups;
  } else {
    return all_rounds();
  }
}

// Returns an array mapping roundid to a string that encodes the state of the
// schedule for that roundid.  The string is treated as an opaque signature to
// detect when a schedule changes.
function schedule_signature() {
  global $db;
  $result = array();

  // This won't handle a completely deleted round
  $stmt = $db->prepare('SELECT roundid FROM Rounds'
                       .' WHERE NOT EXISTS(SELECT 1 FROM RaceChart WHERE RaceChart.roundid = Rounds.roundid)');
  $stmt->execute(array());
  foreach($stmt as $row) {
    $result[$row['roundid']] = "";
  }

  // Signature comprises resultid and racerid pairs, in heat-lane order.
  // There's a remote chance that two different schedules with byes could
  // somehow have the same signature.
  $stmt = $db->prepare('SELECT Rounds.roundid, round, heat, lane, resultid, racerid'
                       .' FROM Rounds'
                       .' INNER JOIN RaceChart'
                       .' ON RaceChart.roundid = Rounds.roundid'
                       .' ORDER BY Rounds.roundid, Rounds.classid, heat, lane');
  $stmt->execute(array());
  $last_roundid = -1;
  foreach ($stmt as $row) {
    if ($row['roundid'] != $last_roundid) {
      if ($last_roundid >= 0) {
        $result[$last_roundid] = $signature;
      }
      $signature = "";
      $last_roundid = $row['roundid'];
    }
    if (strlen($signature) > 0) {
      $signature .= "/";
    }
    $signature .= $row['resultid']."-".$row['racerid'];
  }
  if ($last_roundid >= 0) {
    $result[$last_roundid] = $signature;
  }

  return $result;
}

?>
