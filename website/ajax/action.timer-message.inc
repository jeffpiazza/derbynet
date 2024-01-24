<?php

require_once('inc/racing-state.inc');
require_once('inc/timer-state.inc');
require_once('inc/write-heat-results.inc');
require_once('inc/autoadvance.inc');
require_once('inc/replay.inc');
require_once('inc/remote-start.inc');
require_once('inc/events.inc');
require_once('inc/timer-flags.inc');
require_once('inc/event-lane-details.inc');
require_once('inc/timer-logging.inc');

// Timer-controller-to-web-service messages:
//
// message="HELLO"
// Initial message from timer controller
//
// message="IDENTIFIED" & lane_count= & timer= & human= & ident= & options=
// Sent when timer controller discovers/identifies the timer.
// timer= is the Java class name of the TimerDevice class that recognizes the
//     timer.
// human= is the human-friendly name of the device/manufacturer
// ident= is the specific identifier string (if any) from the timer (serial number, etc.).
// options= is any extra detail about this particular timer, in a comma-separated key:value pairs
//
// message="STARTED"
// Sent when gate opens
//
// message="FINISHED" & lane1="3.123"&place1="1"&..., etc.
//
// message="HEARTBEAT" & confirmed=0|1 (healthy implied) & unhealthy
// Unhealthy means the timer is (still) unhealthy; this message should have been preceded by
//  a HELLO (for initial search) or MALFUNCTION message.
// If unhealthy not present, then confirmed=1 means the timer has sent recognizable data.
//
// message="MALFUNCTION" & detectable=0|1 & error= message
//   Signals lost connection or other problem with the timer
//
// message="FLAGS" & many parameters
//   Reports ports, devices, and flag settings from the timer jar

// Web-service-to-timer-controller messages:
//
// <HEAT-READY lane-mask="15"/> -- prepare timer (15 = 1111b)
// <ABORT/> -- reset timer, abandon 

$remote_start = 'NO';
if (isset($_POST['remote-start'])) {
  $remote_start = $_POST['remote-start'];
}

if (isset($_POST['overdue'])) {
  record_event_to_limit(EVENT_TIMER_OVERDUE, 1000, array('other' => $_POST['overdue']));
}

$message = $_POST['message'];

$racing_state = get_racing_state();  // True if we're racing, false if not
$timer_state = new TimerState();     // An object carrying our idea of the timer's current state

start_response();

if (have_permission(TIMER_MESSAGE_PERMISSION)) {
  set_has_remote_start($remote_start == 'YES');

  if ($message == 'HELLO') {
    // Timer begins in TIMER_SEARCHING, almost the same as unhealthy, until we
    // receive confirmation that derby-timer.jar has actually connected to the
    // timer device.  Otherwise, all we know is that derby-timer.jar is alive
    // and scanning.
    $timer_state->to_state(TIMER_SEARCHING);
    // Upon getting a new timer connection, choose a new name for server-side
    // timer log file (which we may well never create).
    set_timer_log_file();
    if (isset($_POST['interface'])) {
      write_raceinfo('timer-interface',
                     isset($_POST['build'])
                     ? $_POST['interface'].'-'.$_POST['build']
                     : $_POST['interface']);
    }
    $last_contact = $timer_state->last_contact();
    $accept = true;
    if ($last_contact >= 0 && $last_contact + 1 >= time()) {
      $ua = read_raceinfo('timer-agent');
      if ($ua && $ua != $_SERVER['HTTP_USER_AGENT']) {
        $accept = false;
        echo "<competing>".htmlspecialchars($ua, ENT_QUOTES, 'UTF-8')."</competing>\n";
      }
    }
    // Record the user-agent string of the interface that most recently established contact.
    // Previously-established interfaces will get <competing/> messages.
    if ($accept) {
      write_raceinfo('timer-agent', $_SERVER['HTTP_USER_AGENT']);
    }
    echo "\n  <success/>\n";
    echo "    <query/>\n";
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
    if (isset($_POST['human'])) {
      write_raceinfo('timer-human', $_POST['human']);
    }
    if (isset($_POST['ident'])) {
      write_raceinfo('timer-ident', $_POST['ident']);
    }
    if (isset($_POST['options'])) {
      write_raceinfo('timer-options', $_POST['options']);
    }
    if (@$_POST['vid']) {
      write_raceinfo('timer-vid', $_POST['vid']);
    }
    if (@$_POST['pid']) {
      write_raceinfo('timer-pid', $_POST['pid']);
    }
    if (isset($_POST['confirmed']) && $_POST['confirmed'] == 0) {
      $timer_state->to_state(TIMER_UNCONFIRMED);
    } else {
      $timer_state->to_state(TIMER_CONNECTED);
    }
    echo "\n  <success/>\n";
  } else if ($message == 'STARTED') {
    $timer_state->to_state(TIMER_RUNNING);
    send_replay_RACE_STARTS();
    echo "\n  <success/>\n";
  } else if ($message == 'FINISHED') {
    $nlanes = get_lane_count();
    $results = parse_heat_results($nlanes);
    if (read_raceinfo_boolean('reverse-lanes')) {
      $oldresults = $results;  // Copies the array
      for ($i = 0; $i < $nlanes; ++$i) {
        $results[$i] = $oldresults[$nlanes - $i - 1];
      }
    }
    if (!$racing_state) {
      // TODO This might be a test run, or an actual race run while the "racing"
      // switch was off.  We should prompt the race coordinator and offer to
      // record these times somewhere.
      echo "\n  <failure code='notracing'>Not racing now.</failure>\n";
      write_raceinfo('refused-results', 1 + read_raceinfo('refused-results', 0));
      record_timer_finish(EVENT_TIMER_RESULT_REFUSED, $results, null);
    } else {
      if (write_heat_results($results)) {
        send_replay_REPLAY();
        write_raceinfo('timer_current', '');
        $current = get_running_round();
        advance_heat($current, 'next', /* show failure */ false);
        $racing_state = get_racing_state();
        record_timer_finish(EVENT_HEAT_RESULT_FROM_TIMER, $results, $current);
        echo "\n  <success/>\n";
      }
    }
    // If another heat is teed up, we'll immediately change state to
    // TIMER_STAGING, below.
    $timer_state->to_state(TIMER_CONNECTED);
  } else if ($message == 'HEARTBEAT') {
    echo "\n  <success/>\n";
    if (isset($_POST['unhealthy'])) {
      // Probably a no-op
      if (!$timer_state->unhealthy()) {
        $timer_state->to_state(TIMER_SEARCHING);
      }
    } else if (isset($_POST['confirmed'])) {
      if ($_POST['confirmed'] == 0) {
        $timer_state->to_state(TIMER_UNCONFIRMED);
      } else {
        // #218: Don't let "confirmed" flip TIMER_STAGING or TIMER_RUNNING back to TIMER_CONNECTED
        $tstate = $timer_state->state();
        if ($tstate == TIMER_NOT_CONNECTED || $tstate == TIMER_SEARCHING || $tstate == TIMER_UNCONFIRMED) {
          $timer_state->to_state(TIMER_CONNECTED);
        }
      }
    } else if ($timer_state->unhealthy()) {
      // A good heartbeat converts unhealthy back to connected
      $timer_state->to_state(TIMER_CONNECTED);
    }
    $ua = read_raceinfo('timer-agent');
    if ($ua && $ua != $_SERVER['HTTP_USER_AGENT']) {
      echo "<competing>".htmlspecialchars($ua, ENT_QUOTES, 'UTF-8')."</competing>\n";
    }
  } else if ($message == 'MALFUNCTION') {
    $error_string = isset($_POST['error']) ? $_POST['error'] : "Malfunction";
    if (!$timer_state->troubled()) {
      record_event(EVENT_TIMER_MALFUNCTION, array('other' => $error_string));
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
  } else if ($message == 'FLAGS') {
    decode_timer_flags($_POST);
    echo "\n  <success/>\n";
  } else {
    echo "\n  <failure code='notunderstood'>Message "
        .htmlspecialchars($message, ENT_QUOTES, 'UTF-8')
        ." not recognized.</failure>\n";
  }

  $timer_current = read_raceinfo('timer_current', '');
  $current = get_running_round();
  if ($timer_current) {
    $timer_current = explode('#', $timer_current);
    if ($current['roundid'] != $timer_current[0] ||
        $current['heat'] != $timer_current[1]) {
      echo "\n <abort />\n";
      if ($timer_state->state() != TIMER_UNCONFIRMED) {
        $timer_state->to_state(TIMER_CONNECTED);
      }
      write_raceinfo('timer_current', '');
    }
  }

  $tstate = $timer_state->state();
  if ($racing_state && ($tstate == TIMER_CONNECTED || $tstate == TIMER_UNCONFIRMED)) {
    // i.e., not staging or running
    $pending = true;
    $mask = 0;
    $nlanes = get_lane_count();
    if ($current['roundid'] == TIMER_TEST_ROUNDID) {
      $mask = (1 << $nlanes) - 1 - read_raceinfo('tt-mask', 0);
    } else {
      $stmt = $db->prepare('SELECT lane, finishtime, finishplace FROM RaceChart'
                           .' WHERE roundid = :roundid AND heat = :heat'
                           .' ORDER BY lane');
      $stmt->execute(array(':roundid' => $current['roundid'],
                           ':heat' => $current['heat']));
      foreach ($stmt as $row) {
        if ($row['finishtime'] || $row['finishplace']) {
          $pending = false;
        }
        $mask |= (1 << ($row['lane'] - 1));
      }
    }
    if (read_raceinfo_boolean('reverse-lanes')) {
      $oldmask = $mask;
      $mask = 0;
      for ($l = 0; $l < $nlanes; ++$l) {
        if (($oldmask & (1 << $l)) != 0) {
          $mask |= (1 << ($nlanes - $l - 1));
        }
      }
    }
    if ($pending) {
      echo '  <heat-ready lane-mask="'.$mask.'"'
          .' class="'.htmlspecialchars($current['class'], ENT_QUOTES, 'UTF-8').'"'
          .' round="'.$current['round'].'"'
          .' roundid="'.$current['roundid'].'"'
          .' heat="'.$current['heat'].'"'
          .' lanes="'.$nlanes.'"'
          .'/>'."\n";
      if ($tstate == TIMER_CONNECTED) {
        $timer_state->to_state(TIMER_STAGING);
      }
      // send_replay_START($current);
      write_raceinfo('timer_current', $current['roundid'].'#'.$current['heat']);
    }
  }

  send_timer_assignments();
  
  if (remote_start_pending()) {
    echo '  <remote-start/>'."\n";
  }

  echo "<remote-log send='".(read_raceinfo_boolean('timer-send-logs') ? "true" : "false")."'/>\n";
  
  if (!$racing_state && ($tstate == TIMER_STAGING || $tstate == TIMER_RUNNING)) {
    echo "\n  <abort/>\n";
    $timer_state->to_state(TIMER_CONNECTED);
    write_raceinfo('timer_current', '');
  }
  $timer_state->write(true);
} else {
	not_authorized_body();
}

end_response();

?>
