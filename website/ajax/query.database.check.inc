<?php

// Validates some expected invariants in the database and reports any
// discrepancies.  Reports success or failure as an outcome, even though it's a
// query and not an action.

$ok = true;

function check_empty($tag, $sql) {
  global $db;
  global $ok;

  $failures = $db->query($sql)->fetchAll(PDO::FETCH_ASSOC);
  if (count($failures) > 0) {
    json_out($tag, $failures);
    $ok = false;
  }
}

////////////////////// Awards

check_empty('awardid-awardtypeid',
            'SELECT * FROM Awards WHERE awardtypeid NOT IN (SELECT awardtypeid FROM AwardTypes)');

if (false) {  // These are frequently violated
check_empty('awardid-classid',
            'SELECT * FROM Awards WHERE classid IS NOT NULL AND classid <> 0'
            .' AND classid NOT IN (SELECT classid FROM Classes)');

check_empty('awardid-rankid',
            'SELECT * FROM Awards WHERE rankid IS NOT NULL AND rankid <> 0'
            .' AND rankid NOT IN (SELECT rankid FROM Ranks)');
}

check_empty('award-racerid',
            'SELECT * FROM Awards WHERE racerid IS NOT NULL AND racerid <> 0'
            .' AND racerid NOT IN (SELECT racerid FROM RegistrationInfo)');

////////////////////// 
// TODO audit-table

////////////////////// 
// TODO events-table

////////////////////// Classes
if (false) {
  // Organic classes: must have at least one rank (unless there are no
  // partitions at all for the class)
  check_empty('classes-ranks',
              'SELECT * FROM Classes'
              .' WHERE constituents = \'\' AND rankids = \'\''
              .'  AND classid NOT IN (SELECT DISTINCT classid FROM Ranks)');
}
// Every organic class has a round 1
check_empty('classes-round1',
            'SELECT * FROM Classes'
            .' WHERE constituents = \'\' AND rankids = \'\''
            .'  AND classid NOT IN (SELECT DISTINCT classid FROM Rounds WHERE round = 1)');

// Aggregate classes: can't have members or ranks
check_empty('aggregate-members',
            'SELECT * FROM Classes'
            .' WHERE (constituents <> \'\' OR rankids <> \'\')'
            .'  AND classid IN (SELECT DISTINCT classid FROM RegistrationInfo)');
check_empty('aggregate-ranks',
            'SELECT * FROM Classes'
            .' WHERE (constituents <> \'\' OR rankids <> \'\')'
            .'  AND classid IN (SELECT DISTINCT classid FROM Ranks)');

check_empty('classes-constituent',
            'SELECT * FROM Classes WHERE constituents <> \'\''
            .' AND classid NOT IN (SELECT DISTINCT dependent FROM ConstituentClasses)');
check_empty('classes-sortorder',
            'SELECT * FROM Classes WHERE sortorder IS NULL');
check_empty('classes-ambiguous-sort',
            'SELECT sortorder, COUNT(*) FROM Classes'
            .' GROUP BY sortorder'
            .' HAVING COUNT(*) > 1');

////////////////////// ConstituentClasses
check_empty('constituentclasses-antecedent',
            'SELECT * FROM ConstituentClasses'
            .' WHERE antecedent NOT IN (SELECT DISTINCT classid FROM Classes)');
check_empty('constituentclasses-dependent',
            'SELECT * FROM ConstituentClasses'
            .' WHERE dependent NOT IN (SELECT DISTINCT classid FROM Classes WHERE constituents <> \'\')');


////////////////////// RaceChart
// It's not obvious RaceChart.classid has to be populated always.
// RaceChart.classid is redundant to Rounds.classid
check_empty('racechart-classid',
            'SELECT * FROM RaceChart'
            .' WHERE classid <> (SELECT classid FROM Rounds WHERE Rounds.roundid = RaceChart.roundid)');

check_empty('racechart-roundid',
            'SELECT * FROM RaceChart WHERE roundid NOT IN (SELECT roundid FROM Rounds)');

check_empty('racechart-masterheat',
            'SELECT * FROM RaceChart AS rc1 WHERE masterheat <> 0'
            .' AND EXISTS(SELECT 1 FROM RaceChart AS rc2'
            .'            WHERE rc1.roundid = rc2.roundid AND rc1.heat = rc2.heat'
            .'              AND rc1.masterheat <> rc2.masterheat)');
check_empty('racechart-masterheat2',
            'SELECT * FROM RaceChart AS rc1 WHERE masterheat <> 0'
            .' AND (rc1.completed IS NULL OR rc1.completed = \'\')'
            .' AND EXISTS(SELECT 1 FROM RaceChart AS rc2'
            .'            WHERE rc1.masterheat = rc2.masterheat'
            .'             AND (rc2.completed IS NULL OR rc2.completed = \'\')'
            .'             AND (rc1.roundid <> rc2.roundid OR rc1.heat <> rc2.heat))'
            .' ORDER BY masterheat, roundid, heat');

check_empty('racechart-straight-lanes',
            'SELECT roundid, heat, COUNT(*)'
            .' FROM RaceChart'
            .' GROUP BY roundid, heat'
            .' HAVING COUNT(*) > (SELECT COALESCE(itemvalue, 6) FROM RaceInfo'
            .'                    WHERE itemkey = \'lane_count\')');
check_empty('racechart-one-car-per-lane',
            'SELECT roundid, heat, lane, COUNT(*)'
            .' FROM RaceChart'
            .' GROUP BY roundid, heat, lane'
            .' HAVING COUNT(*) > 1');
check_empty('racechart-lane',
            'SELECT * FROM RaceChart'
            .' WHERE lane IS NULL OR lane = \'\'');


////////////////////// RaceInfo

////////////////////// Ranks
check_empty('ranks-classid',
            'SELECT * FROM Ranks WHERE classid NOT IN (SELECT classid FROM Classes)');
check_empty('ranks-sortorder',
            'SELECT * FROM Ranks WHERE sortorder IS NULL');
check_empty('ranks-ambiguous-sort',
            'SELECT classid, sortorder, COUNT(*) FROM Ranks'
            .' GROUP BY classid, sortorder'
            .' HAVING COUNT(*) > 1');

////////////////////// RegistrationInfo
check_empty('racer-partitionid',
            'SELECT * FROM RegistrationInfo'
            .' WHERE partitionid NOT IN (SELECT partitionid FROM Partitions)');
check_empty('racer-classid',
            'SELECT * FROM RegistrationInfo'
            .' WHERE classid NOT IN (SELECT classid FROM Classes)');
check_empty('racer-rankid',
            'SELECT * FROM RegistrationInfo'
            .' WHERE rankid NOT IN (SELECT rankid FROM Ranks)');

////////////////////// Roster
check_empty('roster-roundid',
            'SELECT * FROM Roster WHERE roundid NOT IN (SELECT roundid FROM Rounds)');

// Roster classid is redundant to Rounds.classid
check_empty('roster-classid',
            'SELECT * FROM Roster'
            .' WHERE classid <> (SELECT classid FROM Rounds WHERE Rounds.roundid = Roster.roundid)');
check_empty('roster-racerid',
            'SELECT * FROM Roster'
            .' WHERE racerid NOT IN (SELECT racerid FROM RegistrationInfo)');
// This only works for first-round rosters!!
check_empty('roster-duplicates',
            'SELECT racerid, classid, COUNT(*) FROM Roster'
            .' WHERE roundid IN (SELECT roundid FROM Rounds WHERE round = 1)'
            .' GROUP BY racerid, classid '
            .'HAVING COUNT(*) > 1');
// Each racer should appear in the roster for the first round of the corresponding group
check_empty('roster-round-classid',
            'SELECT * FROM RegistrationInfo JOIN Rounds USING (classid)'
            .' JOIN Roster USING (racerid)'
            .' WHERE round = 1'
            .'  AND RegistrationInfo.classid <> Roster.classid');

////////////////////// Rounds
check_empty('rounds-sequential',
            'SELECT * FROM Rounds AS rnext'
            .' WHERE round <> 1'
            .'  AND NOT EXISTS(SELECT 1 FROM Rounds AS rprev'
            .'                 WHERE rnext.classid = rprev.classid'
            .'                  AND rnext.round = rprev.round + 1)');
check_empty('rounds-classid',
            'SELECT * FROM Rounds'
            .' WHERE classid NOT IN (SELECT classid FROM Classes)');

////////////////////// Kiosks
////////////////////// MessageQueue
////////////////////// Scenes

////////////////////// SceneKiosk
check_empty('scenekiosk-scene',
            'SELECT * FROM SceneKiosk'
            .' WHERE sceneid NOT IN (SELECT sceneid FROM Scenes)');

////////////////////// Playlist
check_empty('playlist-seq',
            'SELECT * FROM Playlist WHERE seq IS NULL');
check_empty('playlist-ambiguous-seq',
            'SELECT seq, COUNT(*) FROM Playlist'
            .' GROUP BY seq'
            .' HAVING COUNT(*) > 1');
check_empty('playlist-classid',
            'SELECT * FROM Playlist'
            .' WHERE classid NOT IN (SELECT classid FROM Classes)');
check_empty('playlist-round',
            'SELECT * FROM Playlist WHERE round is NULL');

////////////////////// BallotAwards
////////////////////// Voters
////////////////////// Votes

////////////////////// Partitions
check_empty('partition-rankid',
            'SELECT * FROM Partitions WHERE rankid NOT IN (SELECT rankid FROM Ranks)');
check_empty('partition-sortorder',
            'SELECT * FROM Partitions WHERE sortorder IS NULL');




if ($ok) {
  json_success();
} else {
  json_failure('corrupt', 'Discrepancies detected');
}

?>
