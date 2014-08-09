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
        .' name="'.htmlspecialchars($row['firstname'].' '.$row['lastname'], ENT_QUOTES).'"'
        .' carname="'.htmlspecialchars($row['carname'], ENT_QUOTES).'"'
       .' carnumber="'.$row['carnumber'].'"'
       .' photo="'.htmlspecialchars($row['imagefile'], ENT_QUOTES).'"'
       .' finishtime="'.($row['finishtime'] ? sprintf('%5.3f', $row['finishtime']) : '').'"'
       .'/>'."\n";
}
?>