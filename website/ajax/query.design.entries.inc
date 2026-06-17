<?php
// $_GET['racerid'] - The racer to get entries for

$racerid = $_GET['racerid'];

$stmt = $db->prepare('SELECT de.awardid, a.awardname 
                     FROM DesignEntries de
                     INNER JOIN Awards a ON de.awardid = a.awardid
                     WHERE de.racerid = :racerid');
$stmt->execute(array(':racerid' => $racerid));

$entries = array();
foreach ($stmt as $row) {
  $entries[] = array(
    'awardid' => $row['awardid'],
    'awardname' => $row['awardname']
  );
}

json_out('entries', $entries);
json_success();
?>
