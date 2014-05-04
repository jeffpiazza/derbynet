<?php
require_once('data.inc');

// Collect the top N racer IDs in each class
function top_finishers_by_class($n_per_class) {
  global $conn;
  $top_finishers = array(); // ClassID -> array(racerIDs) (1st,2nd,3rd)
  $sql = 'SELECT RegistrationInfo.RacerID, RegistrationInfo.ClassID, count(*) as base, sum(FinishTime)/count(*) as avg'
	.' FROM (RegistrationInfo'
	.' INNER JOIN RaceChart'
	.' ON RaceChart.RacerID = RegistrationInfo.RacerID)'
	.' WHERE PassedInspection = 1 AND Exclude = 0'
	.' AND FinishTime IS NOT NULL'
	.' GROUP BY RegistrationInfo.RacerID, RegistrationInfo.ClassID'
	//  .' HAVING count(*) = '.$nlanes -- racers who finished all their heats
	.' ORDER BY RegistrationInfo.ClassID, sum(FinishTime)/count(*)';

  $rs = odbc_exec($conn, $sql);

  while (odbc_fetch_row($rs)) {
	$classid = odbc_result($rs, 'ClassID');
	$racerid = odbc_result($rs, 'RacerID');
	$avg = odbc_result($rs, 'avg');
	if (!isset($top_finishers[$classid]))
	  $top_finishers[$classid] = array();
	if (count($top_finishers[$classid]) < $n_per_class) {
	  $top_finishers[$classid][] = $racerid;
	}
  }

  return $top_finishers;
}

function top_finishers_overall($n_trophies) {
  global $conn;
  $results = Array();

  if ($n_trophies > 0) {
	$sql = 'SELECT TOP '.$n_trophies.' RaceChart.RacerID'
	  .' FROM (RegistrationInfo'
	  .' INNER JOIN RaceChart'
	  .' ON RaceChart.RacerID = RegistrationInfo.RacerID)'
	  .' WHERE PassedInspection = 1 AND Exclude = 0'
	  .' AND FinishTime IS NOT NULL'
	  .' GROUP BY RaceChart.RacerID'
	  //    .' HAVING count(*) = '.$nlanes
	  .' ORDER BY sum(FinishTime)/count(*)';

	$rs = odbc_exec($conn, $sql);

	while (odbc_fetch_row($rs)) {
	  $results[] = odbc_result($rs, 'RacerID');
	}
  }

  return $results;
}



?>