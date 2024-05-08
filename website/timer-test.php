<?php @session_start();

require_once('inc/data.inc');
session_write_close();
require_once('inc/banner.inc');
require_once('inc/locked.inc');
require_once('inc/json-current-heat.inc');
require_once('inc/json-timer-state.inc');
require_once('inc/timer-test.inc');

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
<link rel="stylesheet" type="text/css" href="css/timer-test.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/mobile.js"></script>
<script type="text/javascript" src="js/ajax-setup.js"></script>
<script type="text/javascript" src="js/modal.js"></script>
<script type="text/javascript" src="js/timer-test.js"></script>
<script type="text/javascript">
$(function() {
    update_timer_summary(<?php echo json_encode(json_timer_state(), JSON_HEX_TAG | JSON_HEX_AMP); ?>,
                         <?php echo json_encode(json_timer_details(), JSON_HEX_TAG | JSON_HEX_AMP); ?>,
                         <?php echo json_encode(json_current_heat($current), JSON_HEX_TAG | JSON_HEX_AMP); ?>);
});

</script>
</head>
<body>
<?php make_banner('Timer Testing', 'coordinator.php'); ?>

<div id="log_outer">
  <div class="test-control">
    <label for="timer-send-logs">Remote logging:</label>
    <input id="timer-send-logs" type="checkbox" class="flipswitch" <?php
    if (read_raceinfo_boolean('timer-send-logs')) {
      echo " checked='checked'";
    } ?>/>
    <span id="log-location"><?php
        if (!locked_settings()) {
          echo htmlspecialchars(read_raceinfo('timer-log', ''), ENT_QUOTES, 'UTF-8');
        } ?></span>
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
           type="checkbox"<?php if (read_raceinfo_boolean('reverse-lanes')) echo ' checked="checked"';?>/>
    <label id="reverse-lanes-label" for="reverse-lanes">Number lanes in reverse</label>
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

  <div id="timer_settings_button_div" class="block_buttons">
    <input type="button" value="Timer Settings" onclick="handle_timer_settings_button()"/>
  </div>

  <div id="start_race_button_div" class="block_buttons hidden">
    <input type="button" value="Start Race" onclick="handle_start_race_button()"/>
  </div>

  <div id="fake_timer_div" class="block_buttons hidden">
    <a class="button_link" href="fake-timer.php" target="_blank">Fake Timer</a>
  </div>

</div>

<div id='timer_settings_modal' class='modal_dialog wide_modal hidden block_buttons'>
  <div id='timer_settings_port_and_device'>
    <div id='timer_settings_port'>
      <select></select>
    </div>
    <div id='timer_settings_device'>
      <select></select>
    </div>
  </div>

  <table id="timer_settings_modal_flags"></table>
  <input type="button" value="Cancel"
      onclick='close_modal("#timer_settings_modal");'/>
</div>

</body>
</html>
