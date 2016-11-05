<now-racing>
<?php
// action.php?query=poll.now-racing&roundid= &heat= 
// Returns:
//   <now-racing roundid= heat= >
//     <timer-trouble/>   if there's a problem with the timer
//     <heat-result lane="1" time="" place="" speed=""/>... if results are available
//     <current-heat ...> 
//     - if current-heat differs from what the caller passed in, then provide:
//     <racer lane="1" name="Jimmy Jones" carname="Greased Lightning" carnumber="" photo=""/>
//     - if requesting a repeat of the finish-order animation:
//      <repeat-animation/>
//   </now-racing>

if (!with_gprm()) {
  require_once('inc/timer-state.inc');
  $timer_state = new TimerState();
  if ($timer_state->troubled()) {
    echo "  <timer-trouble/>\n";
  }
}

require_once('inc/replay.inc');

$roundid = isset($_GET['roundid']) ? $_GET['roundid'] : '0';
$heat = isset($_GET['heat']) ? $_GET['heat'] : '1';
$track_length = read_raceinfo('track-length', 40);  // in feet

// $track_length / $finishtime gives feet per second.
//  * 3600 gives feet per hour
//  / 5280 gives miles per hour
//  * 25 gives scale miles per mile (not user-adjustable; makes a 7" derby car scale to 14.5 feet)
$fps_to_scale_mph = 3600 * 25 / 5280;

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
  $speed = $finishtime == 0 ? "--" : sprintf("%4.1f", $track_length / $finishtime * $fps_to_scale_mph);
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

if (read_raceinfo('replay_just_finished')) {
  write_raceinfo('replay_just_finished', 0);
  echo "<repeat-animation/>\n";
} else if (replay_playing()) {
  echo "<hold-current-screen/>\n";
}
?>
</now-racing>
