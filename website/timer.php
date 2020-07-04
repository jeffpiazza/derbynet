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
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/ajax-setup.js"></script>
<script type="text/javascript" src="js/timer.js"></script>
<script type="text/javascript">
$(function() {
  var parser = new DOMParser();
  var xmlDoc = parser.parseFromString(
    "<doc><timer-state icon='<?php echo $timer_state_status[1]; ?>'>"
    + <?php echo json_encode($timer_state_status[0]); ?>
    + "</timer-state></doc>", "text/xml");
  update_timer_status(xmlDoc.getElementsByTagName('timer-state')[0]);
});

</script>
<style>
#log_container {
  float: right;
  border: 2px solid black;
  width: 50%;
  overflow-y: scroll;
  margin-top: 10px;
}

div#timer_block {
  display: inline-block;
}
div#timer_status h3 {
  margin-top: 0px;
  margin-bottom: 5px;
}
div#timer_status p {
  margin-top: 5px;
}
img#timer_status_icon {
  float: left;
  margin-left: 20px;
  margin-right: 20px;
}

table#lanes {
  margin-left: 20px;
}

table#lanes td {
  min-width: 80px;
}
table#lanes td.time {
  text-align: right;
}
</style>
</head>
<body>
<?php make_banner('Timer', 'coordinator.php'); ?>

<div id="log_container">
  <pre id="log_text">
  </pre>
</div>

  <label style="font-size: 20px;" for="n-lanes">Number of lanes on the track:</label>
  <input id="n-lanes" name="n-lanes" type="number" min="0" max="20"
  style="line-height: 1.3; font-family: sans-serif; font-size: 24px"
         data-enhanced="true"
         value="<?php echo get_lane_count(); ?>"/>

  <input type="hidden" id="unused-lane-mask" name="unused-lane-mask"
           value="<?php echo read_raceinfo('tt-mask', read_raceinfo('unused-lane-mask', 0)); ?>"/>

<div id="timer_status">
  <img id="timer_status_icon" src="img/status/unknown.png"/>
  <div id="timer_block">
    <h3>Timer Status</h3>
    <p><b id="timer_status_text">Timer status not yet updated</b></p>
    <div id="timer-details"></div>
  </div>
</div>

<table id="lanes" style="font-size: 24px;">
  <tr><th>Lane</th>
      <th>Occupied?</th>
      <th>Time</th>
      <th>Place</th>
  </tr>
</table>

<div>
<input type="checkbox" id="test-mode" <?php
    if ($current['roundid'] == TIMER_TEST_ROUNDID && $current['now_racing']) {
      echo " checked=\"checked\"";
    } ?>/>
<label for="test-mode">Test mode</label>
</div>

<form id="log-setting-form" action="action.php">
  <input type="hidden" name="action" value="settings.write"/>
  <?php $logging_on = read_raceinfo_boolean('timer-send-logs'); ?>
  <p>Remote Logging</p>
  <input type="radio" name="timer-send-logs" id="remote-logs-on"
      value="1" <?php if ($logging_on) echo " checked='checked'"; ?>/>
  <label for="remote-logs-on">On</label>
  <br/>
  <input type="radio" name="timer-send-logs" id="remote-logs-off"
      value="0" <?php if (!$logging_on) echo " checked='checked'"; ?>/>
  <label for="remote-logs-off">Off</label>
</form>
</body>
</html>
