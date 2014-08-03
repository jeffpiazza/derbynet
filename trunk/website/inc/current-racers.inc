<?php
// Generate XML elements for the racers in the current heat
// Expects $now_running array with roundid and heat elements

require_once('inc/data.inc');
  
$stmt = $db->prepare('SELECT lane, lastname, firstname, carnumber, carname, imagefile, finishtime'
                     .' FROM RaceChart'
                     .' INNER JOIN RegistrationInfo'
                     .' ON RaceChart.racerid = RegistrationInfo.racerid'
                     .' WHERE roundid = :roundid'
                     .' AND heat = :heat'
                     .' ORDER BY lane');
$stmt->execute(array(':roundid' => $now_running['roundid'],
                     ':heat' => $now_running['heat']));
foreach ($stmt as $row) {
  echo '  <racer lane="'.$row['lane'].'"'
          // TODO: Offer different formats for name
          // TODO: escape as XML text
       .' name="'.$row['firstname'].' '.$row['lastname'].'"'
       .' carname="'.$row['carname'].'"'
       .' carnumber="'.$row['carnumber'].'"'
       .' photo="'.$row['imagefile'].'"'
       .' finishtime="'.($row['finishtime'] ? sprintf('%5.3f', $row['finishtime']) : '').'"'
       .'/>'."\n";
}
?>