<?php session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_permission(SET_UP_PERMISSION);  // TODO: What's the correct permission?
?>
<html>
<head>
<title>Race Coordinator Page</title>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<link rel="stylesheet" type="text/css" href="css/coordinator.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript">
// We're using jQuery Mobile for its nice mobile/tablet-friendly UI
// elements.  By default, jQM also wants to hijack page loading
// functionality, and perform page loads via ajax, a "service" we
// don't really want.  Fortunately, it's possible to turn that stuff
// off.
$(document).bind("mobileinit", function() {
				   $.extend($.mobile, {
					 ajaxEnabled: false
					 });
				 });
</script>
<script type="text/javascript" src="js/jquery.mobile-1.4.2.min.js"></script>
<script type="text/javascript" src="js/coordinator.js"></script>
</head>
<body>
<?php $banner_title = 'Race Dashboard'; require('inc/banner.inc'); ?>

<div class="double_control_column">

<div id="now-racing-group" class="scheduling_control_group">
  <p>Waiting for coordinator-poll query...</p>
</div>

</div>

<div class="control_column">

<div class="control_group heat_control_group">
  <div class="centered_flipswitch">
    <input type="checkbox" data-role="flipswitch" name="is-currently-racing" id="is-currently-racing"
      checked="checked"
      data-on-text="Racing" data-off-text="Not Racing"/>
  </div>

  <div class="block_buttons">
    <input type="button" data-enhanced="true" value="Skip Heat" onclick="handle_skip_heat_button()"/><br/>
    <input type="button" data-enhanced="true" value="Previous Heat" onclick="handle_previous_heat_button()"/><br/>
    <input type="button" data-enhanced="true" value="Manual Results" onclick="show_manual_results_modal()"/><br/>
    <!-- TODO: discard results (one or all heats) -->
  </div>

</div>


<div class="control_group timer_control_group">
  <h3>Timer Status</h3>
  <p><b id="timer_status_text">Timer status not yet updated</b></p>
  <p>The track has <span id="lane_count">an unknown number of</span> lane(s).</p>
</div>

<div class="control_group replay_control_group">
  <h3>Replay Status</h3>
  <p><b id="replay_status">Remote replay status not yet updated</b></p>
  <div id="test_replay" class="block_buttons">
    <input type="button" data-enhanced="true" value="Test Replay" onclick="handle_test_replay();"/>
  </div>
</div>

<div id="kiosk_control_group" class="kiosk_control_group">
</div>

</div>

<div class="control_column">

<div id="ready-to-race-group" class="scheduling_control_group"></div>
<div id="not-yet-scheduled-group" class="scheduling_control_group"></div>
<div id="done-racing-group" class="scheduling_control_group"></div>

<?php
// TODO: Controls for creating 2nd round, grand finals round, including roster
?>
</div>

<div id='modal_background'></div>

<div id='kiosk_modal' class="modal_dialog hidden block_buttons">
  <form>
    <label for="kiosk_name_field">Name for kiosk:</label>
    <input type="text" id="kiosk_name_field"/>
    <input type="submit" data-enhanced="true" value="Assign"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick='close_kiosk_modal();'/>
  </form>
</div>

<div id='schedule_modal' class="modal_dialog hidden block_buttons">
  <form>
    <p>How many times should each racer appear in each lane?</p>
    <select id="schedule_num_rounds">
        <option>1</option>
        <option>2</option>
        <option>3</option>
        <option>4</option>
        <option>5</option>
        <option>6</option>
    </select>

    <input type="submit" data-enhanced="true" value="Schedule"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick='close_schedule_modal();'/>
  </form>
</div>

<div id='manual_results_modal' class="modal_dialog hidden block_buttons">
  <form>
    <input type="hidden" name="action" value="heat-results"/>
    <table></table>
    <input type="button" data-enhanced="true"
           id="discard-results"
           onclick='handle_discard_results_button();'
           value="Discard Results"/>
    <input type="submit" data-enhanced="true" value="Change"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick='close_manual_results_modal();'/>
  </form>
</div>

</body>
</html>