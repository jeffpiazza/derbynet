<now-racing>
<?php
// action.php?query=poll.now-racing&roundid= &heat= row-height=
//  Passed-in roundid & heat affect what heat-results, if any, are returned.
//    (That's the only thing they affect, though.)
//  row-height is used for constructing photo URLs, if any, so photos will be
//    the right size.
//
// Returns:
//   <now-racing roundid= heat= >
//     <timer-trouble/>   if there's a problem with the timer
//     <heat-result lane="1" time="" place="" speed=""/>... if results are available
//     <current-heat ...> 
//     <racer lane="1" name="Jimmy Jones" carname="Greased Lightning" carnumber="" photo=""/>
//     - if the server is requesting a repeat of the finish-order animation:
//     <repeat-animation/>
//   </now-racing>

if (warn_no_timer()) {
  require_once('inc/timer-state.inc');
  $timer_state = new TimerState();
  if ($timer_state->troubled()) {
    echo "  <timer-trouble/>\n";
  }
}

require_once('inc/replay.inc');
require_once('inc/current-racers.inc');
require_once('inc/current-heat.inc');
require_once('inc/photos-on-now-racing.inc');

$roundid = isset($_GET['roundid']) ? $_GET['roundid'] : '0';
$heat = isset($_GET['heat']) ? $_GET['heat'] : '1';
$track_length = read_raceinfo('track-length', 40);  // in feet
$time_format = get_finishtime_formatting_string();

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
                     .' AND (finishtime IS NOT NULL OR finishplace IS NOT NULL)'
                     .' ORDER BY lane');
$stmt->execute(array(':roundid' => $roundid,
                     ':heat' => $heat));
foreach ($stmt as $row) {
  $finishtime = $row['finishtime'];
  $speed = $finishtime == 0 ? "--" : sprintf("%4.1f", $track_length / $finishtime * $fps_to_scale_mph);
  echo '  <heat-result lane="'.$row['lane'].'"';
  echo ' time="'.sprintf($time_format, $finishtime).'"';
  echo ' place="'.$row['finishplace'].'"';
  echo ' speed="'.$speed.'"';
  echo "/>\n";
}

// This lets the js reserve room for the right number of digits
echo "<zero zero='".sprintf($time_format, 0.0)."'/>\n";

$now_running = get_running_round();
$use_master_sched = use_master_sched();
emit_current_heat($now_running, $use_master_sched);

$render = false;
if (isset($_GET['row-height']) && $_GET['row-height'] > 0) {
  $photos = read_photos_on_now_racing();
  if ($photos) {
    $render_name = (2 * $_GET['row-height']).'x'.$_GET['row-height'];
    $render = photo_repository($photos)->lookup_or_any($render_name);
  }
}

emit_current_racers($now_running, read_raceinfo('name-style', FULL_NAME), $render);

if (read_raceinfo('replay_just_finished')) {
  write_raceinfo('replay_just_finished', 0);
  echo "<repeat-animation/>\n";
} else if (replay_playing()) {
  echo "<hold-current-screen/>\n";
}
?>
</now-racing>
