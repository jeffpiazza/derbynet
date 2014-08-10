<watching>
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
$track_length = read_raceinfo('track_length', 50);  // in feet

// TODO: Refine scale_mph_factor
$scale_mph_factor = 12.0;  // Kinda-sorta converts fps into scale MPH

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
  $finishtime = $row['finishtime'];
  $speed = $finishtime == 0 ? "--" : sprintf("%4.1f", $track_length / $finishtime * $scale_mph_factor);
  echo '  <heat-result lane="'.$row['lane'].'"';
  // TODO: Controllable number of decimal places?
  echo ' time="'.sprintf('%5.3f', $finishtime).'"';
  echo ' place="'.$row['finishplace'].'"';
  echo ' speed="'.$speed.'"';
  echo "/>\n";
}

require_once('inc/current-heat.inc');

if (($now_running['roundid'] != $roundid || $now_running['heat'] != $heat) && $now_running['now_racing'] != 0) {
  require('inc/current-racers.inc');
}
?>
</watching>
