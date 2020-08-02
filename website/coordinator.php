<?php session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/banner.inc');
require_once('inc/authorize.inc');
require_permission(SET_UP_PERMISSION);  // TODO: What's the correct permission?
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Race Coordinator Page</title>
<link rel="stylesheet" type="text/css" href="css/jquery-ui.min.css"/>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/ajax-setup.js"></script>
<script type="text/javascript" src="js/jquery-ui.min.js"></script>
<link rel="stylesheet" type="text/css" href="css/coordinator.css"/>
<script type="text/javascript" src="js/mobile-init.js"></script>
<!-- For flipswitch and select elements: -->
<script type="text/javascript" src="js/jquery.mobile-1.4.2.min.js"></script>
<script type="text/javascript" src="js/dashboard-ajax.js"></script>
<script type="text/javascript" src="js/modal.js"></script>
<script type="text/javascript" src="js/wrap-flipswitch.js"></script>
<script type="text/javascript" src="js/coordinator-controls.js"></script>
<script type="text/javascript" src="js/coordinator-poll.js"></script>
</head>
<body>
<?php make_banner('Race Dashboard'); ?>
<?php require_once('inc/ajax-failure.inc'); ?>

<div class="double_control_column">
  <div id="now-racing-group" class="scheduling_control_group">
    <p>Waiting for poll.coordinator query...</p>
  </div>
</div>

<div class="control_column_container">

<div class="control_column">

  <div class="control_group heat_control_group">
    <div id="start_race_button_div" class="block_buttons hidden">
      <input type="button" data-enhanced="true" value="Start Race" onclick="handle_start_race_button()"/>
    </div>
    <div class="centered_flipswitch">
      <input type="checkbox" data-role="flipswitch" name="is-currently-racing" id="is-currently-racing"
        checked="checked"
        data-on-text="Racing" data-off-text="Not Racing"/>
    </div>

    <div class="block_buttons">

      <div id="prev_heat_button" class="button_link" onclick="handle_previous_heat_button()">
        <img src="img/left-white-60.png"/>
      </div>

      <input type="button" id="manual_results_button" data-enhanced="true" value="Manual Results"
        onclick="show_manual_results_modal()" />

      <div id="skip_heat_button" class="button_link" onclick="handle_skip_heat_button()">
        <img src="img/right-white-60.png"/>
      </div>

      <input type="button" id="rerun-button" data-enhanced="true" value="Re-Run"
             data-rerun="none"
             onclick="handle_rerun(this);"/>
    </div>
  </div>

  <div id="supplemental-control-group" class="control_group block_buttons new_round_control hidden">
      <div id="add-new-rounds-button" class="hidden">
          <input type="button" data-enhanced="true" value="Add New Rounds"
                 onclick="show_choose_new_round_modal()"/>
      </div>
      <div id="now-racing-group-buttons"></div>
  </div>

  <div class="control_group timer_control_group">
  <?php if (!warn_no_timer()) { ?>
    <p>Not monitoring timer state</p>
   <?php } else { ?>
    <div class="status_icon">
      <img id="timer_status_icon" src="img/status/unknown.png"/>
    </div>

    <div id='timer-test' class="block_buttons">
      <a class='button_link' href='timer.php'>Test</a>
    </div>

    <h3>Timer Status</h3>
    <p><b id="timer_status_text">Timer status not yet updated</b></p>
    <p>The track has <span id="lane_count">an unknown number of</span> lane(s).</p>
   <?php } ?>
  </div>

  <div class="control_group replay_control_group">
    <div class="status_icon">
      <img id="replay_status_icon" src="img/status/unknown.png"/>
    </div>
    <h3>Replay Status</h3>
    <p><b id="replay_status">Remote replay status not yet updated</b></p>
    <div id="test_replay" class="block_buttons">
      <input type="button" data-enhanced="true" value="Trigger Replay" onclick="handle_test_replay();"/>
    </div>
    <div class="block_buttons">
      <input type="button" data-enhanced="true" value="Replay Settings" onclick="show_replay_settings_modal();"/>
    </div>
  </div>

</div>

<div class="control_column">

  <div id="master-schedule-group" class="master_schedule_group"></div>
  <div id="ready-to-race-group" class="scheduling_control_group"></div>
  <div id="not-yet-scheduled-group" class="scheduling_control_group"></div>
  <div id="done-racing-group" class="scheduling_control_group"></div>

</div>

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

    <input id="schedule-and-race"
           type="submit" data-enhanced="true" value="Schedule + Race"
           data-race="true" onclick='mark_clicked($(this));'/>
    <input id="schedule-only"
           type="submit" data-enhanced="true" value="Schedule Only"
           data-race="false" onclick='mark_clicked($(this));'/>
    <input type="button" data-enhanced="true" value="Cancel"
           onclick='close_modal("#schedule_modal");'/>
  </form>
</div>

<div id='manual_results_modal' class="modal_dialog hidden block_buttons">
  <form>
    <input type="hidden" name="action" value="result.write"/>
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
    <input type="hidden" name="action" value="settings.write"/>
<?php /*
    <label for="replay-skipback">Duration of replay, in seconds:</label>
    <!-- Could be any decimal value... -->
    <!-- TODO MacReplay only accepts integral skipback values presently -->
    <!-- TODO When displaying this modal, should read the current settings and populate controls accordingly. -->
    <select id="replay-skipback" name="replay-skipback">
        <option value="2">2.0</option>
        <!-- <option>2.5</option> -->
        <option value="3">3.0</option>
        <!-- <option>3.5</option> -->
        <option selected="selected" value="4">4.0</option>
        <!-- <option>4.5</option> -->
        <option value="5">5.0</option>
        <!-- <option>5.5</option> -->
        <option value="6">6.0</option>
        <!-- <option>6.5</option> -->
    </select>
*/ ?>
    <label for="replay-num-showings">Number of times to show replay:</label>
    <!-- Could be any positive integer -->
    <select id="replay-num-showings" name="replay-num-showings">
        <option>1</option>
        <option selected="selected">2</option>
        <option>3</option>
    </select>

    <label for="replay_rate">Replay playback speed:</label>
    <!-- Could be any decimal value -->
    <select id="replay-rate" name="replay-rate">
        <option value="0.10">0.1x</option>
        <option value="0.25">0.25x</option>
        <option value="0.50">0.5x</option>
        <option selected="selected" value="0.75">0.75x</option>
        <option value="1.00">1x</option>
    </select>
    <input type="submit" data-enhanced="true" value="Submit"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick='close_modal("#replay_settings_modal");'/>
  </form>
</div>

<div id='choose_new_round_modal' class="modal_dialog block_buttons hidden">
  <!-- Populated by script: see offer_new_rounds() -->
</div>

<div id='new_round_modal' class="modal_dialog block_buttons hidden">
  <form>
    <input type="hidden" name="action" value="roster.new"/>
    <input type="hidden" name="roundid" id="new_round_roundid"/>

    <div id="multi_flipswitches" class="multi_den_only">
    </div>

    <p>Choose top</p>
    <input type="number" name="top" id="now_round_rop" value="3"/>

    <div class="single_den_only">
    <?php if (use_subgroups()) { ?>
      <p>racers from</p>
      <div class="centered_flipswitch">
        <input type="checkbox" data-role="flipswitch" name="bucketed" id="bucketed_single"
               data-on-text="Each <?php echo subgroup_label(); ?>" data-off-text="Overall"/>
      </div>
    <?php } else { ?>
      <p>racers</p>
    <?php } ?>
    </div>

    <div class="multi_den_only">
      <p>racers from</p>
      <div class="centered_flipswitch">
        <input type="checkbox" data-role="flipswitch" name="bucketed" id="bucketed_multi"
               data-on-text="Each <?php echo group_label(); ?>" data-off-text="Overall"/>
      </div>
       <p>Name for new round:</p>
       <input type="text" name="classname" id="agg_classname" value="Grand Finals"/>
    </div>

    <input type="submit" data-enhanced="true" value="Submit"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick='g_new_round_modal_open = false; close_modal("#new_round_modal");'/>
  </form>
</div>

<div id='unschedule_modal' class="modal_dialog hidden block_buttons">
  <p>Round <span id="unschedule_round"></span> for <span id="unschedule_class"></span>
       has a schedule, but no heats have been run.  If you want to add or remove racers,
       you need to delete the schedule for this round.  Is that what you would like to do?</p>
  <form>
    <input type="submit" data-enhanced="true" value="Unschedule"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick='close_modal("#unschedule_modal");'/>
  </form>
</div>

                                                                                            
<div id="purge_modal" class="modal_dialog wide_modal block_buttons hidden">
    <div id="purge_modal_inner">
  <form>
    <h3><span id="purge_round_name"></span> Round <span id="purge_round_no"></span></h3>
    <p>This round has <span id="purge_results_count"></span> heat(s) with results.</p>
    <input type="submit" data-enhanced="true" value="Purge Results"/>

    <p>&nbsp;</p>
    <input type="button" data-enhanced="true" value="Cancel"
        onclick='close_modal("#purge_modal");'/>
  </form>
    </div>
</div>

<div id="purge_confirmation_modal" class="modal_dialog block_buttons hidden">
  <form>
    <p>You are about to purge all the race results for this round.
       This operation cannot be undone.
       Are you sure that's what you want to do?</p>

    <input type="submit" data-enhanced="true" value="Purge Results"/>

    <p>&nbsp;</p>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick='close_secondary_modal("#purge_confirmation_modal");'/>
  </form>
</div>
                                                                                            
<div id='delete_round_modal' class="modal_dialog hidden block_buttons">
  <p>Round <span id="delete_round_round"></span> for <span id="delete_round_class"></span>
       has no schedule, and no heats have been run.  To choose different racers for this round,
       you need to delete this round and generate a new one.  Is that what you would like to do?</p>
  <form>
    <input type="submit" data-enhanced="true" value="Delete Round"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick='close_modal("#delete_round_modal");'/>
  </form>
</div>
</body>
</html>
