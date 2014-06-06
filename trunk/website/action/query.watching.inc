<?php
// action.php?query=watching&roundid= &heat= 
// Returns:
//   <watching roundid= heat= >
//     <heat-result lane="1" time="" place="" speed=""/>... if results are available
//     <current-heat> 
//     -- if current-heat differs from what the caller passed in, then provide
//     --    <racer lane="1" name="Jimmy Jones" carname="Greased Lightning" carnumber="" photo=""/>
//   </watching>

$roundid = isset($_GET['roundid']) ? $_GET['roundid'] : '0';
$heat = isset($_GET['heat']) ? $_GET['heat'] : '1';
?>
<watching>
<?php
    $stmt = $db->prepare('SELECT lane, finishtime, finishplace, completed'
                         .' FROM RaceChart'
                         .' WHERE roundid = :roundid'
                         .' AND heat = :heat'
                         //.' AND completed IS NOT NULL AND completed != \'\''
                         .' AND finishtime IS NOT NULL'
                         .' ORDER BY lane');
    $stmt->execute(array(':roundid' => $roundid,
                         ':heat' => $heat));
    foreach ($stmt as $row) {
        echo '  <heat-result lane="'.$row['lane'].'" time="'.$row['finishtime'].'"'
             .' place="'.$row['finishplace'].'"'
             // TODO: Calculate corny speed
             .' speed=""/>'."\n";
    }
?>
  <?php
require('action/current-heat.inc');

if ($now_running['roundid'] != $roundid || $now_running['heat'] != $heat) {
    $stmt = $db->prepare('SELECT lane, lastname, firstname, carnumber, carname, imagefile'
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
             .' photo="'.$row['imagefile'].'"/>'."\n";
    }
}
 ?>
</watching>
