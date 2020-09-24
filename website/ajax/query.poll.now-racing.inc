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
require_once('inc/heat-results.inc');

$roundid = isset($_GET['roundid']) ? $_GET['roundid'] : '0';
$heat = isset($_GET['heat']) ? $_GET['heat'] : '1';
emit_heat_results($roundid, $heat);

// This lets the js reserve room for the right number of digits
echo "<zero zero='".sprintf($time_format, 0.0)."'/>\n";

$now_running = get_running_round();
emit_current_heat($now_running);

$render = false;
if (isset($_GET['row-height']) && $_GET['row-height'] > 0) {
  $photos = read_photos_on_now_racing();
  if ($photos) {
    $render_name = (2 * $_GET['row-height']).'x'.$_GET['row-height'];
    $render = photo_repository($photos)->lookup_or_any($render_name);
  }
}

emit_current_racers($now_running, read_raceinfo('name-style', FULL_NAME), $render);
?>
</now-racing>
