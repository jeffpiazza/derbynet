<?php

require_once('inc/racing-state.inc');
require_once('inc/timer-state.inc');
require_once('inc/write-heat-results.inc');
require_once('inc/autoadvance.inc');
require_once('inc/replay.inc');
require_once('inc/events.inc');

// Timer-controller-to-web-service messages:
//
// message="HELLO"
// Initial message from timer controller
//
// message="IDENTIFIED" & lane_count= & timer= & ident= & options=
// Sent when timer controller discovers/identifies the timer.
// timer= is the Java class name of the TimerDevice class that recognizes the
//     timer.
// ident= is the identifier string (if any) from the timer.
// options= is any extra detail about this particular timer.
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

$racing_state = get_racing_state();  // True if we're racing, false if not
$timer_state = new TimerState();     // An object carrying our idea of the timer's current state

start_response();

if (have_permission(TIMER_MESSAGE_PERMISSION)) {
  if ($message == 'HELLO') {
    // Timer begins in TIMER_SEARCHING, almost the same as unhealthy, until we
    // receive confirmation that derby-timer.jar has actually connected to the
    // timer device.  Otherwise, all we know is that derby-timer.jar is alive
    // and scanning.
    $timer_state->to_state(TIMER_SEARCHING);
    echo "\n  <success/>\n";
  } else if ($message == 'IDENTIFIED') {
    if (isset($_POST['lane_count']) && $_POST['lane_count'] > 0) {
      write_raceinfo('reported_lane_count', $_POST['lane_count']);
      // TODO: If the timer disagrees with the user as to lane
      // count, who should prevail?
      write_raceinfo('lane_count', $_POST['lane_count']);
    }
    if (isset($_POST['timer'])) {
      write_raceinfo('timer-type', $_POST['timer']);
    }
    if (isset($_POST['ident'])) {
      write_raceinfo('timer-ident', $_POST['ident']);
    }
    if (isset($_POST['options'])) {
      write_raceinfo('timer-options', $_POST['options']);
    }
    $timer_state->to_state(TIMER_CONNECTED);
    echo "\n  <success/>\n";
  } else if ($message == 'STARTED') {
    $timer_state->to_state(TIMER_RUNNING);
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
      advance_heat($current, 'next', /* show failure */ false);
      $racing_state = get_racing_state();
      echo "\n  <success/>\n";
    }
    // If another heat is teed up, we'll immediately change state to
    // TIMER_STAGING, below.
    $timer_state->to_state(TIMER_CONNECTED);
  } else if ($message == 'HEARTBEAT') {
    echo "\n  <success/>\n";
    // TODO: Capture other state information
    if ($timer_state->unhealthy()) {
      $timer_state->to_state(TIMER_CONNECTED);
    }
  } else if ($message == 'MALFUNCTION') {
    $error_string = isset($_POST['error']) ? $_POST['error'] : "Malfunction";
    if (!$timer_state->troubled()) {
      record_event_sql('INSERT INTO Events(tstamp, action, other)'
                       .' VALUES('.dbnow_expr().', :action, :other)',
                       array(':action' => EVENT_TIMER_MALFUNCTION,
                             ':other' => $error_string));
    }
    echo "\n  <success/>\n";
    if ($timer_state->state() == TIMER_NOT_CONNECTED) {
      $timer_state->to_state(TIMER_CONNECTED);
    }
    if (isset($_POST['detectable']) && $_POST['detectable'] == 0) {
      // TODO Duration should maybe come from the malfunction message?
      $timer_state->set_malfunction(60, $error_string);
    } else {
      $timer_state->set_unhealthy(isset($_POST['error']) ? $_POST['error'] : "Malfunction");
    }
    set_racing_state(false);
  } else {
    echo "\n  <failure code='notunderstood'>Message "
        .htmlspecialchars($message, ENT_QUOTES, 'UTF-8')
        ." not recognized.</failure>\n";
  }

  $tstate = $timer_state->state();
  if ($racing_state && $tstate == TIMER_CONNECTED) {  // i.e., not staging or running
    $current = get_running_round();
    $stmt = $db->prepare('SELECT lane, finishtime FROM RaceChart'
                         .' WHERE roundid = :roundid AND heat = :heat'
                         .' ORDER BY lane');
    $stmt->execute(array(':roundid' => $current['roundid'],
                         ':heat' => $current['heat']));
    $pending = true;
    $mask = 0;
    foreach ($stmt as $row) {
      if ($row['finishtime'] || $row['finishplace']) {
        $pending = false;
      }
      $mask |= (1 << ($row['lane'] - 1));
    }
    if ($pending) {
      echo '  <heat-ready lane-mask="'.$mask.'"'
          .' class="'.htmlspecialchars($current['class'], ENT_QUOTES, 'UTF-8').'"'
          .' round="'.$current['round'].'"'
          .' roundid="'.$current['roundid'].'"'
          .' heat="'.$current['heat'].'"'
          .'/>'."\n";
      $timer_state->to_state(TIMER_STAGING);
      // send_replay_START($current);
    }
  }

  if (!$racing_state && ($tstate == TIMER_STAGING || $tstate == TIMER_RUNNING)) {
    echo "\n  <abort/>\n";
    $timer_state->to_state(TIMER_CONNECTED);
  }
  $timer_state->write(true);
} else {
	not_authorized_body();
}

end_response();

?>