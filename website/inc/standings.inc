<?php
require_once('inc/data.inc');

// Returns a list of roundids for the highest-numbered round for each class with
// participants.
function final_roundids() {
  global $db;
  $stmt = $db->query('SELECT R2.roundid AS roundid'
                     .' FROM (SELECT Rounds.classid, MAX(round) AS max_round'
                     .' FROM Rounds'
                     // The join with RegistrationInfo is to filter out the
                     // Grand Finals class, which has no registered members.
                     .' INNER JOIN RegistrationInfo'
                     .' ON Rounds.classid = RegistrationInfo.classid'
                     .' GROUP BY Rounds.classid) AS R1'
                     .' INNER JOIN Rounds AS R2'
                     .' ON R1.classid = R2.classid'
                     .' AND R1.max_round = R2.round');
  $final_roundids = array();
  foreach ($stmt as $row) {
	$final_roundids[] = $row['roundid'];
  }
  return $final_roundids;
}

// $roundids is a string of comma-separated roundid values.
// Alternatively, a single numeric roundid works, too.
function standings_for_rounds($roundids) {
  global $db;

  $scoring_expr = read_raceinfo_boolean('drop-slowest') 
      ? '(SUM(finishtime) - MAX(finishtime))/(COUNT(*) - 1)'
      : 'SUM(finishtime)/COUNT(*)';

  // We want this "overall" set of results, ordered by time score.  For any other rounds (Rounds.roundid NOT IN (0,...)),
  // we're going to need to place the values in a separate map by roundid.
  $stmt = $db->query('SELECT Rounds.roundid, RegistrationInfo.racerid, RegistrationInfo.classid,'
                     .' class, firstname, lastname, carnumber,'
                     .' Rounds.roundid IN (0,'.$roundids.') AS final,'
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
                      'final' => $row['final'],
                      'classid' => $row['classid'],
                      'class' => $row['class'],
                      'firstname' => $row['firstname'],
                      'lastname' => $row['lastname'],
                      'carnumber' => $row['carnumber'],
                      'base' => $row['base'],  // Number of heats run in round
                      // TODO: Showing negative for preliminary-round standings
                      'time' => $row['avg'],
                      'best' => $row['best'],
                      'worst' => $row['worst']);
  }

  return $result;
}

?>