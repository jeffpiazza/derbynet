<?php @session_start();

require_once('inc/banner.inc');
require_once('inc/data.inc');
require_once('inc/timer-state-xml.inc');

$current = get_running_round();

// $timer_state_status comprises two values. This is just the initial value; the
// page will poll for updates
$timer_state_status = expand_timer_state_status(new TimerState());
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Timer</title>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/mobile.css"/>
<link rel="stylesheet" type="text/css" href="css/timer.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/mobile.js"></script>
<script type="text/javascript" src="js/ajax-setup.js"></script>
<script type="text/javascript" src="js/timer.js"></script>
<script type="text/javascript">
$(function() {
  var parser = new DOMParser();
  var xmlDoc = parser.parseFromString(
    "<doc><timer-state icon='<?php echo $timer_state_status[1]; ?>'>"
    + <?php echo json_encode($timer_state_status[0]); ?>
    + "</timer-state></doc>", "text/xml");
  update_timer_summary(xmlDoc.getElementsByTagName('timer-state')[0]);
});

</script>
</head>
<body>
<?php make_banner('Timer', 'coordinator.php'); ?>

<div id="log_outer">
  <div class="test-control">
    <label for="timer-send-logs">Remote logging:</label>
    <input id="timer-send-logs" type="checkbox" class="flipswitch" <?php
    if (read_raceinfo_boolean('timer-send-logs')) {
      echo " checked='checked'";
    } ?>/>
  </div>

  <div id="log_container">
    <pre id="log_text"></pre>
  </div>
</div>

<div id="timer_summary">
  <img id="timer_summary_icon" src="img/status/unknown.png"/>
  <div id="timer_summary_text">
    <h3>Timer Status</h3>
    <p><b id="timer_status_text">Timer status not yet updated</b></p>
    <div id="timer-details"></div>
  </div>
</div>

<div class="test-control">
  <label for="test-mode">Simulate racing:</label>
  <input id="test-mode" type="checkbox" class="flipswitch" <?php
    if ($current['roundid'] == TIMER_TEST_ROUNDID && $current['now_racing']) {
      echo " checked=\"checked\"";
    } ?>/>
</div>

<div id="now-racing">
  <label id="n-lanes-label" for="n-lanes">Number of lanes on the track:</label>
  <input id="n-lanes" name="n-lanes" type="number" class="not-mobile" min="0" max="20"
         value="<?php echo get_lane_count(); ?>"/>
  <div>
    <input id="reverse-lanes" name="reverse-lanes" class="not-mobile"
           style="margin-left: 50px; "
           type="checkbox"<?php if (read_raceinfo_boolean('reverse-lanes')) echo ' checked="checked"';?>/>
    <label style="font-size: 18px;" for="reverse-lanes">Number lanes in reverse</label>
  </div>

  <input type="hidden" id="unused-lane-mask" name="unused-lane-mask"
           value="<?php echo read_raceinfo('tt-mask', read_raceinfo('unused-lane-mask', 0)); ?>"/>

  <table id="lanes">
    <tr><th>Lane</th>
        <th>Occupied?</th>
        <th>Time</th>
        <th>Place</th>
    </tr>
  </table>
</div>

</body>
</html>
