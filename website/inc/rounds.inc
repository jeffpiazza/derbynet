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

// Returns each distinct Rounds entry:
//  {roundid, round, classid, class, groupid(=roundid), roundname}
function all_rounds() {
  global $db;
  $use_groups = use_groups();
  $rounds = array();
  foreach ($db->query('SELECT roundid, round, Classes.classid, class'
					  .' FROM Rounds INNER JOIN Classes'
					  .' ON Rounds.classid = Classes.classid'
					  .' ORDER BY '
                      .(schema_version() >= 2 ? 'Classes.sortorder, ' : '')
                      .'class, round') as $row) {
    $rounds[] = array('roundid' => $row['roundid'],
					  'round' => $row['round'],
					  'classid' => $row['classid'],
					  'class' => $row['class'],
                      'groupid' => $row['roundid'],
					  'roundname' => $use_groups
                          ? $row['class'].', Round '.$row['round']
                          : ('Round '.$row['round']));
  }

  return $rounds;
}

// Each entry has:
//  {roundid, round, classid, class, groupid(=roundid), roundname,
//    aggregate(0 or 1),
//    roster_size, passed, unscheduled,
//    heats_scheduled, heats_run}
function all_rounds_with_counts() {
  $rounds = all_rounds();
  foreach ($rounds as $round) {
    $roundid = $round['roundid'];
    $round['aggregate'] = is_aggregate_class($round) ? 1 : 0;
    // Number of racers in the roster for the round
    $round['roster_size'] = read_single_value('SELECT COUNT(*) FROM Roster WHERE roundid = :roundid',
                                              array(':roundid' => $roundid));
    // Number of racers in the round who've passed inspection.  (After
    // the first round, this should be the same as the roster size.)
    $round['passed'] = read_single_value('SELECT COUNT(*) FROM Roster'
                                         .' INNER JOIN RegistrationInfo'
                                         .' ON Roster.racerid = RegistrationInfo.racerid'
                                         .' WHERE roundid = :roundid'
                                         .' AND RegistrationInfo.passedinspection <> 0',
                                         array(':roundid' => $roundid));
    // Number of passed racers in the round who aren't in the schedule for the round
    $round['unscheduled'] = read_single_value('SELECT COUNT(*)'
                                              .' FROM Roster'
                                              .' INNER JOIN RegistrationInfo'
                                              .' ON Roster.racerid = RegistrationInfo.racerid'
                                              .' WHERE Roster.roundid = :roundid'
                                              .' AND RegistrationInfo.passedinspection <> 0'
                                              .' AND NOT EXISTS(SELECT 1 FROM RaceChart'
                                              .'  WHERE RaceChart.roundid = Roster.roundid'
                                              .'  AND RaceChart.racerid = Roster.racerid)',
                                              array(':roundid' => $roundid));
    $round['heats_scheduled'] = read_single_value('SELECT COUNT(*) FROM'
                                                  .' (SELECT DISTINCT heat'
                                                  .'   FROM RaceChart'
                                                  .'   WHERE roundid = :roundid) AS tmp',
                                                  array(':roundid' => $roundid));
    $round['heats_run'] = read_single_value('SELECT COUNT(*) FROM'
                                            .' (SELECT DISTINCT heat'
                                            .'   FROM RaceChart'
                                            .'   WHERE roundid = :roundid'
                                            .'   AND (finishtime IS NOT NULL'
                                            .'         OR finishplace IS NOT NULL)) AS tmp',
                                            array(':roundid' => $roundid));
  }
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

function write_signatures() {
  foreach (schedule_signature() as $roundid => $signature) {
    echo "<round roundid='".$roundid."'>".$signature."</round>\n";
  }
}

?>