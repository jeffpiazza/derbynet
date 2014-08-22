<?php session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_permission(SET_UP_PERMISSION);  // TODO: What's the correct permission?
?>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Race Coordinator Page</title>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<link rel="stylesheet" type="text/css" href="css/coordinator.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery-ui-1.10.4.min.js"></script>
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
<!-- For flipswitch and select elements: -->
<script type="text/javascript" src="js/jquery.mobile-1.4.2.min.js"></script>
<script type="text/javascript" src="js/coordinator.js"></script>
</head>
<body>
<?php $banner_title = 'Race Dashboard'; require('inc/banner.inc'); ?>
<?php require_once('inc/ajax-failure.inc'); ?>
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
  <div class="status_icon">
    <img id="timer_status_icon" src="img/status_unknown.png"/>
  </div>
  <h3>Timer Status</h3>
  <p><b id="timer_status_text">Timer status not yet updated</b></p>
  <p>The track has <span id="lane_count">an unknown number of</span> lane(s).</p>
</div>

<div class="control_group replay_control_group">
  <div class="status_icon">
    <img id="replay_status_icon" src="img/status_unknown.png"/>
  </div>
  <h3>Replay Status</h3>
  <p><b id="replay_status">Remote replay status not yet updated</b></p>
  <div id="test_replay" class="block_buttons">
    <input type="button" data-enhanced="true" value="Test Replay" onclick="handle_test_replay();"/>
  </div>
  <div class="block_buttons">
    <input type="button" data-enhanced="true" value="Settings" onclick="show_replay_settings_modal();"/>
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
      onclick='close_modal("#kiosk_modal");'/>
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
      onclick='close_modal("#schedule_modal");'/>
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
      onclick='close_modal("#manual_results_modal");'/>
  </form>
</div>

<div id='replay_settings_modal' class="modal_dialog hidden block_buttons">
  <form>
    <input type="hidden" name="action" value="change-replay"/>

    <label for="replay_duration">Duration of replay, in seconds:</label>
    <!-- Could be any decimal value... -->
    <select id="replay_duration" name="replay_duration">
        <option>2.0</option>
        <option>2.5</option>
        <option>3.0</option>
        <option>3.5</option>
        <option>4.0</option>
        <option>4.5</option>
        <option>5.0</option>
        <option>5.5</option>
        <option>6.0</option>
        <option>6.5</option>
    </select>

    <label for="replay_num_showings">Number of times to show replay:</label>
    <!-- Could be any positive integer -->
    <select id="replay_num_showings" name="replay_num_showings">
        <option>1</option>
        <option>2</option>
        <option>3</option>
    </select>

    <label for="replay_rate">Replay playback speed:</label>
    <!-- Could be any decimal value -->
    <select id="replay_rate" name="replay_rate">
        <option>0.1x</option>
        <option>0.25x</option>
        <option>0.5x</option>
        <option>0.75x</option>
        <option>1x</option>
    </select>
    <input type="submit" data-enhanced="true" value="Submit"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick='close_modal("#replay_settings_modal");'/>
  </form>
</div>

<div id='new_round_modal' class="modal_dialog hidden block_buttons">
  <form>
    <input type="hidden" name="action" value="make-roster"/>
    <input type="hidden" name="roundid" id="new_round_roundid"/>

    <label for="now_round_top">Choose top</label>
    <select id="new_round_top" name="top">
        <option>2</option>
        <option selected="selected">3</option>
        <option>4</option>
        <option>5</option>
        <option>6</option>
        <option>7</option>
        <option>8</option>
        <option>9</option>
        <option>10</option>
    </select>
<!--
    <input type="checkbox" data-role="flipswitch" name="bucketed" id="bucketed"
      data-on-text="Each Den" data-off-text="Overall"/>
-->

    <input type="submit" data-enhanced="true" value="Submit"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick='close_modal("#new_round_modal");'/>
  </form>
</div>

</body>
</html>