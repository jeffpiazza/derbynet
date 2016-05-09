<?php

require_once('inc/racing-state.inc');
require_once('inc/timer-state.inc');
require_once('inc/write-heat-results.inc');
require_once('inc/autoadvance.inc');
require_once('inc/replay.inc');

// Timer-controller-to-web-service messages:
//
// message="HELLO"
// Initial message from timer controller
//
// message="IDENTIFIED"& lane_count=
// Sent when timer controller discovers timer
//
// message="STARTED"
// Sent when gate opens
//
// message="FINISHED" & lane1="3.123"&place1="1"&..., etc.
//
// message="HEARTBEAT"
//  Response provides message to timer controller
//
// message="MALFUNCTION" & detectable=0|1 & error= message
//   Signals lost connection or other problem with the timer

// Web-service-to-timer-controller messages:
//
// <HEAT-READY lane-mask="15"/> -- prepare timer (15 = 1111b)
// <ABORT/> -- reset timer, abandon 

$message = $_POST['message'];

$racing_state = get_racing_state();

start_response();

if (have_permission(CONTROL_RACE_PERMISSION)) {
  if ($message == 'HELLO') {
    // Timer state begins in TIMER_MALFUNCTION_AUTO until we receive
    // confirmation that derby-timer.jar has actually connected to the timer
    // device.  Otherwise, all we know is that derby-timer.jar is alive and
    // scanning.
    set_timer_state(TIMER_MALFUNCTION_AUTO);
    echo "\n  <success/>\n";
  } else if ($message == 'IDENTIFIED') {
    if (isset($_POST['lane_count']) && $_POST['lane_count'] > 0) {
      write_raceinfo('reported_lane_count', $_POST['lane_count']);
      // TODO: If the timer disagrees with the user as to lane
      // count, who should prevail?
      write_raceinfo('lane_count', $_POST['lane_count']);
    }
    set_timer_state(TIMER_CONNECTED);
    echo "\n  <success/>\n";
  } else if ($message == 'STARTED') {
    set_timer_state(TIMER_RUNNING);
    echo "\n  <success/>\n";
  } else if ($message == 'FINISHED') {
    if (!$racing_state) {
      // TODO This might be a test run, or an actual race run while the "racing"
      // switch was off.  We should prompt the race coordinator and offer to
      // record these times somewhere.
      echo "\n  <failure code='notracing'>Not racing now.</failure>\n";
    } else if (write_heat_results()) {
      send_replay_REPLAY();
      $current = get_running_round();
      advance_heat($current, @$current['roundid'], 'next', /* show failure */ false);
      echo "\n  <success/>\n";
    }
    // If another heat is teed up, we'll immediately change state to
    // TIMER_PREPARED, below.
    set_timer_state(TIMER_CONNECTED);
  } else if ($message == 'HEARTBEAT') {
    echo "\n  <success/>\n";
    // TODO: Capture other state information
    $tstate = get_timer_state();
    if ($tstate == TIMER_NOT_CONNECTED ||
        $tstate == TIMER_MALFUNCTION_AUTO) {
      set_timer_state(TIMER_CONNECTED);
    } else {
      renew_timer_contact();
    }
  } else if ($message == 'MALFUNCTION') {
    echo "\n  <success/>\n";
    if (isset($_POST['detectable']) && $_POST['detectable'] == 0) {
      set_timer_state(TIMER_MALFUNCTION_MANUAL);
    } else {
      set_timer_state(TIMER_MALFUNCTION_AUTO);
    }
    set_racing_state(false);
  } else {
    echo "\n  <failure code='notunderstood'>Message "
        .htmlspecialchars($message, ENT_QUOTES, 'UTF-8')
        ." not recognized.</failure>\n";
  }

  $tstate = get_timer_state();
  if ($racing_state && $tstate == TIMER_CONNECTED) {  // i.e., not prepared or running
    $current = get_running_round();
    $stmt = $db->prepare('SELECT lane, finishtime FROM RaceChart'
                         .' WHERE roundid = :roundid AND heat = :heat'
                         .' ORDER BY lane');
    $stmt->execute(array(':roundid' => $current['roundid'],
                         ':heat' => $current['heat']));
    $pending = true;
    $mask = 0;
    foreach ($stmt as $row) {
      if ($row['finishtime']) {
        $pending = false;
      }
      $mask |= (1 << ($row['lane'] - 1));
    }
    if ($pending) {
      echo '  <heat-ready lane-mask="'.$mask.'"'
          .' class="'.htmlspecialchars($current['class'], ENT_QUOTES, 'UTF-8').'"'
          .' round="'.$current['round'].'"'
          .' heat="'.$current['heat'].'"'
          .'/>'."\n";
      set_timer_state(TIMER_PREPARED);
      // send_replay_START($current);
    }
  }

  if (!$racing_state && ($tstate == TIMER_PREPARED || $tstate == TIMER_RUNNING)) {
    echo "\n  <abort/>\n";
    set_timer_state(TIMER_CONNECTED);
  }
} else {
	not_authorized_body();
}

end_response();

?>