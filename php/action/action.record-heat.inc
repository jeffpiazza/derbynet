<?php
// $_POST['lane1'], $_POST['lane2'], etc. for times
// TODO: Optionally, $_POST['lane1place'], $_POST['lane2place'], etc.

// TODO: Permissions!
// TODO: Relay completion events?

require_once('action/autoadvance.inc');

start_response();

$current = get_running_round();
$roundid = @$current['roundid'];
if (!$roundid || $roundid < 0) {
	echo '<failure code="nocurrentround">No currently-racing round is available!</failure>';
} else {
	// ResultID, ClassID, RoundID, Heat, Lane, RacerID, ChartNumber,
    // FinishTime, FinishPlace, Points, Completed, IgnoreTime, MasterHeat
	$results = array();
	$stmt = $db->prepare('SELECT resultid, lane FROM RaceChart'
						 .' WHERE roundid = :roundid AND heat = :heat'
						 .' ORDER BY lane');
	$stmt->execute(array(':roundid' => $roundid,
						 ':heat' => $current['heat']));
	foreach ($stmt as $row) {
		$results[$row['lane']] = $row['resultid'];
	}
	$stmt->closeCursor();
	foreach ($results as $lane => $resultid) {
		$time = $_POST['lane'.$lane];  // TODO: Missing?
		$stmt = $db->prepare('UPDATE RaceChart'
							 // CURRENT_TIMESTAMP seems to work with $dbtype=access and mysql
							 .' SET finishtime = :time, completed = CURRENT_TIMESTAMP'
							 .' WHERE resultid = :resultid');
		$stmt->execute(array(':time' => $time,
							 ':resultid' => $resultid));
	}

	$current = get_running_round();
	advance_heat($current, @$current['roundid'], 'auto', false);

	echo '<success/>';
}

end_response();