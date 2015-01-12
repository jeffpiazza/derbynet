<?php
require_once('inc/data.inc');

// Collect the top N racer IDs in each class
function top_finishers_by_class($n_per_class, $required_classid = -1) {
  global $db;
  // This could all be one query in mysql or sqlite, but not access.
  $final_rounds = array();
  $stmt = $db->query('SELECT classid, MAX(round) AS max_round'
					 .' FROM Rounds'
					 .' GROUP BY classid');
  foreach ($stmt as $row) {
	$final_rounds[$row['classid']] = $row['max_round'];
  }
  $stmt->closeCursor();

  $top_finishers = array();
  foreach ($final_rounds as $classid => $max_round) {
	if ($required_classid < 0 || $required_classid == $classid) {
	  $stmt = $db->prepare('SELECT RegistrationInfo.racerid,'
						   .' COUNT(*) AS base, SUM(finishtime)/COUNT(*) AS avg'
						   .' FROM '.inner_join('RegistrationInfo', 'RaceChart',
												'RaceChart.racerid = RegistrationInfo.racerid',
												'Rounds',
												'RaceChart.roundid = Rounds.roundid')
						   .' WHERE RegistrationInfo.classid = :classid'
						   // Inclusion of rounds' classid here is to eliminate
						   // 'Grand Finals' rounds, which have a different classid.
						   .' AND Rounds.classid = :classid2'
						   // This last condition ensures we only
						   // evaluate the highest round of
						   // competition for the racer's class.
						   .' AND Rounds.round = :max_round'
						   .' AND passedinspection = 1 AND exclude = 0'
						   .' AND finishtime IS NOT NULL'
						   .' GROUP BY RegistrationInfo.racerid, RegistrationInfo.classid'
						   //.' HAVING COUNT(*) = '.$nlanes -- racers who finished all their heats
						   .' ORDER BY RegistrationInfo.classid, SUM(finishtime)/COUNT(*)');
	  $stmt->execute(array(':classid' => $classid,
						   ':classid2' => $classid,
						   ':max_round' => $max_round));
	  foreach ($stmt as $row) {
		$racerid = $row['racerid'];
		$avg = $row['avg'];
		if (!isset($top_finishers[$classid]))
		  $top_finishers[$classid] = array();
		if (count($top_finishers[$classid]) < $n_per_class) {
		  $top_finishers[$classid][] = $racerid;
		}
	  }

	  $stmt->closeCursor();
	}
  }

  return $top_finishers;
}

function top_finishers_overall($n_trophies) {
  global $db;
  $results = array();

  if ($n_trophies > 0) {
	$stmt = $db->query('SELECT RaceChart.racerid'
					   .' FROM (RegistrationInfo'
					   .' INNER JOIN RaceChart'
					   .' ON RaceChart.racerid = RegistrationInfo.racerid)'
					   .' WHERE passedinspection = 1 AND exclude = 0'
					   .' AND finishtime IS NOT NULL'
					   .' GROUP BY RaceChart.racerid'
					   //.' HAVING COUNT(*) = '.$nlanes
					   .' ORDER BY SUM(finishtime)/COUNT(*)');

	foreach ($stmt as $row) {
	  $results[] = $row['racerid'];
	  if (count($results) >= $n_trophies) {
		  $stmt->closeCursor();
		  break;
	  }
	}
  }

  return $results;
}

?>