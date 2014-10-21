<?php
require_once('inc/data.inc');

// Collect the top N racer IDs in each class
function top_finishers_by_class($n_per_class) {
  global $db;
  $top_finishers = array();
  $stmt = $db->query('SELECT RegistrationInfo.racerid, RegistrationInfo.classid,'
					 .' COUNT(*) AS base, SUM(finishtime)/COUNT(*) AS avg'
					 .' FROM (RegistrationInfo'
					 .' INNER JOIN RaceChart'
					 .' ON RaceChart.racerid = RegistrationInfo.racerid)'
					 .' WHERE passedinspection = 1 AND exclude = 0'
					 .' AND finishtime IS NOT NULL'
					 .' GROUP BY RegistrationInfo.racerid, RegistrationInfo.classid'
					 //.' HAVING COUNT(*) = '.$nlanes -- racers who finished all their heats
					 .' ORDER BY RegistrationInfo.classid, SUM(finishtime)/COUNT(*)');
  foreach ($stmt as $row) {
	$classid = $row['classid'];
	$racerid = $row['racerid'];
	$avg = $row['avg'];
	if (!isset($top_finishers[$classid]))
	  $top_finishers[$classid] = array();
	if (count($top_finishers[$classid]) < $n_per_class) {
	  $top_finishers[$classid][] = $racerid;
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