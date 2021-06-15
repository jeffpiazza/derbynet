<?php @session_start();

require_once('inc/authorize.inc');
require_once('inc/banner.inc');
require_once('inc/data.inc');
require_once('inc/json-current-heat.inc');
require_once('inc/json-timer-state.inc');
require_once('inc/timer-test.inc');
require_permission(SET_UP_PERMISSION);

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
<script type="text/javascript" src="js/modal.js"></script>

<script type="text/javascript" src="js/timer/port_wrapper.js"></script>
<script type="text/javascript" src="js/timer/ports.js"></script>
<script type="text/javascript" src="js/timer/timer-profiles.js"></script>
<script type="text/javascript" src="js/timer/detector.js"></script>
<script type="text/javascript" src="js/timer/events.js"></script>
<script type="text/javascript" src="js/timer/result.js"></script>
<script type="text/javascript" src="js/timer/state_machine.js"></script>
<script type="text/javascript" src="js/timer/timer_proxy.js"></script>
<script type="text/javascript" src="js/timer/host_poller.js"></script>
<script type="text/javascript" src="js/timer/timer-page.js"></script>

</head>
<body>
<?php make_banner('Timer', 'coordinator.php'); ?>

<div id="no-serial-api" class="hidden"
    style="font-weight: bold; font-size: 20px; width: 400px; margin-left: auto; margin-right: auto;">
This browser does not support Web Serial API and so cannot communicate with your timer.
Please try a different browser.
</div>

<div style="width: 400px; float: right; margin-right: 20px;">
  <ul id="profiles-list" class="mlistview">
  </ul>
</div>

<div class="block_buttons" style="width: 400px;">
  <p>Plug in your timer and click "Probe".</p>

  <input id="probe-button" type="button" value="Probe" onclick="probe_for_timer()"/>
  <br/>
  <input id="port-button" type="button" value="New Port" onclick="new_port()"/>
  
</div>

<div style="padding-left: 80px;">
  <p style="font-weight: bold; font-size: 18px; margin-bottom: 3px;" id="connected">Page reloaded</p>
  <p style="margin-top: 0px; margin-bottom: 10px;" id="connected-detail"></p>
</div>

<div id="last_event" style="margin-top: 20px; margin-left: 80px;">
</div>

</body>
</html>
