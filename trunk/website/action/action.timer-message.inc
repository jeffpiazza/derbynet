<?php

require_once('inc/timer-state.inc');
require_once('action/autoadvance.inc');

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
        $current = get_running_round();
        $roundid = @$current['roundid'];
        if (!$roundid || $roundid < 0) {
            echo '<failure code="nocurrentround">No currently-racing round is available!</failure>';
        } else {
            // ResultID, ClassID, RoundID, Heat, Lane, RacerID, ChartNumber,
            // FinishTime, FinishPlace, Points, Completed, IgnoreTime, MasterHeat
            // TODO: Beware of overwriting results!!!
            // TODO: Results from masked-out lanes are a bad sign

            // General strategy should be: reject results, but log
            // them somewhere that can be presented on coordinator
            // dashboard.
            $results = array();
            $stmt = $db->prepare('SELECT resultid, lane FROM RaceChart'
                                 .' WHERE roundid = :roundid AND heat = :heat'
                                 .' ORDER BY lane');
            $stmt->execute(array(':roundid' => $roundid,
                                 ':heat' => $current['heat']));
            foreach ($stmt as $row) {
                $results[$row['lane']] = $row['resultid'];
            }
            $stmt->closeCursor();

            foreach ($results as $lane => $resultid) {
                $time = @$_POST['lane'.$lane];  // TODO: Missing?
                $place = @$_POST['place'.$lane];
                take_action_silently('UPDATE RaceChart'
                                     .' SET finishtime = :time,'
                                     .' finishplace = :place,'
                                     .' completed = '.db_timestamp_expr()
                                     .' WHERE resultid = :resultid',
                                     array(':time' => $time,
                                           ':place' => $place,
                                           ':resultid' => $resultid));
            }

            $current = get_running_round();
            if (advance_heat($current, @$current['roundid'], 'auto', true)) {
                echo '<success/>';
            }
        }
        set_timer_state(TIMER_CONNECTED);
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
        $stmt = $db->prepare('SELECT lane FROM RaceChart'
                             .' WHERE roundid = :roundid AND heat = :heat'
                             .' ORDER BY lane');
        $stmt->execute(array(':roundid' => $current['roundid'],
                             ':heat' => $current['heat']));
        $mask = 0;
        foreach ($stmt as $row) {
            $mask |= (1 << ($row['lane'] - 1));
        }
        echo '<heat-ready lane-mask="'.$mask.'"/>'."\n";
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