<?php

require_once('inc/timer-state.inc');
require_once('inc/write-heat-results.inc');
require_once('inc/autoadvance.inc');

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

start_response();

// TODO: Seems like we want better control over the race sequencing in general.
// E.g., when to prepare for the first race, which round goes next, handling repeated heats.

// As it pertains to the timer, the main issue is when to send a
// heat-ready message, and when to send an abort (e.g. because race
// coordinator decided to repeat or skip a heat, or change rounds).
// The timer cares only inasmuch as it may get a different lane mask
// for a different heat.

if (have_permission(CONTROL_RACE_PERMISSION)) {
    if ($message == 'HELLO') {
        if (isset($_POST['lane_count']) && $_POST['lane_count'] > 0) {
            write_raceinfo('reported_lane_count', $_POST['lane_count']);
            // TODO: If the lane count reported by the timer is wrong
            // for some reason, user input should prevail.  Same
            // database, new timer?
            write_raceinfo('lane_count', $_POST['lane_count']);
        }
        set_timer_state(TIMER_CONNECTED);
        echo '<success/>'."\n";
    } else if ($message == 'STARTED') {
        set_timer_state(TIMER_RUNNING);
        echo '<success/>'."\n";
    } else if ($message == 'FINISHED') {
        if (write_heat_results()) {
            $current = get_running_round();
            if (advance_heat($current, @$current['roundid'], 'next', true)) {
                echo '<success/>';
            }
            set_timer_state(TIMER_CONNECTED);
        }
    } else if ($message == 'HEARTBEAT') {
        echo '<success/>'."\n";
        // TODO: Capture other state information
        if (get_timer_state() == TIMER_DISCONNECTED) {
            set_timer_state(TIMER_CONNECTED);
        }
    } else if ($message == 'MALFUNCTION') {
        echo '<success/>'."\n";
        // TODO: Prepare an alert.  When to clear it?  Need to better
        // define what this means/does.
        set_timer_state(TIMER_CONNECTED);
    } else {
    }

    if (get_timer_state() == TIMER_CONNECTED) {  // i.e., not prepared or running
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
        }
        set_timer_state(TIMER_PREPARED);
        // TODO: Broadcast an event for instant replay
    }
    if (false) {  // TODO
        echo '<abort/>'."\n";
        set_timer_state(TIMER_CONNECTED);
    }
} else {
	not_authorized_body();
}

end_response();

?>