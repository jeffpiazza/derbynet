<?php

require_once('inc/racing-state.inc');
require_once('inc/timer-state.inc');
require_once('inc/write-heat-results.inc');
require_once('inc/autoadvance.inc');
require_once('inc/replay.inc');

// Timer-controller-to-web-service messages:
//
// message="HELLO" & lane_count=
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
// message="MALFUNCTION"
//  Signals operator intervention required

// Web-service-to-timer-controller messages:
//
// <HEAT-READY lane-mask="15"/> -- prepare timer (15 = 1111b)
// <ABORT/> -- reset timer, abandon 

$message = $_POST['message'];

$racing_state = get_racing_state();

start_response();

if (have_permission(CONTROL_RACE_PERMISSION)) {
  if ($message == 'HELLO') {
    if (isset($_POST['lane_count']) && $_POST['lane_count'] > 0) {
      write_raceinfo('reported_lane_count', $_POST['lane_count']);
      // TODO: If the timer disagrees with the user as to lane
      // count, who should prevail?
      write_raceinfo('lane_count', $_POST['lane_count']);
    }
    set_timer_state(TIMER_CONNECTED);
    echo '<success/>'."\n";
  } else if ($message == 'STARTED') {
    set_timer_state(TIMER_RUNNING);
    echo '<success/>'."\n";
  } else if ($message == 'FINISHED') {
    if (!$racing_state) {
      echo "<failure code='notracing'>Not racing now.</failure>\n";
    } else if (write_heat_results()) {
      send_replay_REPLAY();
      $current = get_running_round();
      advance_heat($current, @$current['roundid'], 'next', /* show failure */ false);
      echo '<success/>';
    }
    // If another heat is teed up, we'll immediately change state to
    // TIMER_PREPARED, below.
    set_timer_state(TIMER_CONNECTED);
  } else if ($message == 'HEARTBEAT') {
    echo '<success/>'."\n";
    // TODO: Capture other state information
    if (get_timer_state() == TIMER_NOT_CONNECTED) {
      set_timer_state(TIMER_CONNECTED);
    } else {
      renew_timer_contact();
    }
  } else if ($message == 'MALFUNCTION') {
    echo '<success/>'."\n";
    // TODO: Prepare an alert.  When to clear it?  Need to better
    // define what this means/does.
    set_timer_state(TIMER_CONNECTED);
  } else {
    echo "<failure code='notunderstood'>Message "
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
      echo '<heat-ready lane-mask="'.$mask.'"/>'."\n";
      set_timer_state(TIMER_PREPARED);
      // send_replay_START($current);
    }
  }

  if (!$racing_state && ($tstate == TIMER_PREPARED || $tstate == TIMER_RUNNING)) {
    echo '<abort/>'."\n";
    set_timer_state(TIMER_CONNECTED);
  }
} else {
	not_authorized_body();
}

end_response();

?>