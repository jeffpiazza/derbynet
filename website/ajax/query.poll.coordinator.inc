<coordinator_poll>
<?php
require_once('inc/current-heat.inc');
require_once('inc/current-racers.inc');
require_once('inc/schema_version.inc');

$now_running = get_running_round();
$use_master_sched = use_master_sched();
emit_current_heat($now_running, $use_master_sched);
emit_current_racers($now_running, FULL_NAME, /* photos */ false);
?>
<?php /* Timer */
require_once('inc/timer-state.inc');

$timer_state = new TimerState();
$tstate = $timer_state->state();

if ($tstate == TIMER_NOT_CONNECTED) {
  $msg = 'NOT CONNECTED';
  $icon = 'img/status/not_connected.png';
} else if ($tstate == TIMER_SEARCHING) {
  $msg = 'Scanning for timer';
  $icon = 'img/status/unknown.png';
} else if ($timer_state->troubled()) {
  $msg = $timer_state->malfunction();
  $icon = 'img/status/trouble.png';
} else if ($tstate == TIMER_CONNECTED) {
  $msg ='CONNECTED ('.($timer_state->seconds_since_last_contact()).' sec.)';
  $icon = 'img/status/ok.png';
} else if ($tstate == TIMER_STAGING) {
  $msg = 'Staging';
  $icon = 'img/status/ok.png';
} else if ($tstate == TIMER_RUNNING) {
  $msg = 'Race running';
  $icon = 'img/status/ok.png';
} else {
  $msg = 'Unknown ('.$tstate.')';
  $icon = 'img/status/trouble.png';
}
echo '<timer-state lanes="'.get_lane_count().'"'
    .' last_contact="'.$timer_state->last_contact().'"'
    .' state="'.$tstate.'"'
    .' icon="'.$icon.'"'
    .'>';
echo $msg;
echo '</timer-state>'."\n";
?>
<?php /* Replay */
require_once('inc/replay.inc');
$replay = get_replay();
if ($replay['state'] == REPLAY_NOT_CONNECTED) {
  $msg = 'NOT CONNECTED';
  $icon = 'img/status/not_connected.png';
} else if ($replay['state'] == REPLAY_CONNECTED) {
  $msg = $replay['reported_status'].' ('.(time() - $replay['last_contact']).' sec.)';
  $icon = 'img/status/ok.png';
} else {
  $msg = 'TROUBLE: '.$replay['trouble'];
  $icon = 'img/status/trouble.png';
}
echo '<replay-state'
    .' last_contact="'.$replay['last_contact'].'"'
    .' state="'.$replay['state'].'"'
    .' icon="'.$icon.'"'
    .' connected="'.($replay['state'] != REPLAY_NOT_CONNECTED).'"'
    .'>';
echo $msg;
echo "</replay-state>\n";
?>
<?php // Rounds

$curr_round = $now_running; // Avoid an extra DB call: get_running_round();

$stmt = $db->query('SELECT roundid, Rounds.classid, Classes.class, round FROM Rounds'
                   .' INNER JOIN Classes ON Rounds.classid = Classes.classid'
                   .' ORDER BY round DESC, '
                   .(schema_version() >= 2 ? 'Classes.sortorder, ' : '')
                   .'Classes.class');
$rounds = array();
foreach ($stmt as $round) {
    $rounds[] = $round;
}


foreach ($rounds as $round) {
    $roundid = $round['roundid'];

    echo '<round roundid="'.$roundid.'"'
            .' classid="'.$round['classid'].'"'
            .' class="'.htmlspecialchars($round['class'], ENT_QUOTES, 'UTF-8').'"'
            .' round="'.$round['round'].'"';

    // Number of racers in the roster for the round
    echo ' roster_size="'.read_single_value('SELECT COUNT(*) FROM Roster WHERE roundid = :roundid',
                                            array(':roundid' => $roundid)).'"';

    // Number of racers in the round who've passed inspection.  (After
    // the first round, this should be the same as the roster size.)
    echo ' passed="'.read_single_value('SELECT COUNT(*) FROM Roster'
                                       .' INNER JOIN RegistrationInfo'
                                       .' ON Roster.racerid = RegistrationInfo.racerid'
                                       .' WHERE roundid = :roundid'
                                       .' AND RegistrationInfo.passedinspection <> 0',
                                       array(':roundid' => $roundid)).'"';

    // Number of passed racers in the round who aren't in the schedule for the round
    echo ' unscheduled="'.read_single_value('SELECT COUNT(*)'
                                            .' FROM Roster'
                                            .' INNER JOIN RegistrationInfo'
                                            .' ON Roster.racerid = RegistrationInfo.racerid'
                                            .' WHERE Roster.roundid = :roundid'
                                            .' AND RegistrationInfo.passedinspection <> 0'
                                            .' AND NOT EXISTS(SELECT 1 FROM RaceChart'
                                            .'  WHERE RaceChart.roundid = Roster.roundid'
                                            .'  AND RaceChart.racerid = Roster.racerid)',
                                            array(':roundid' => $roundid)).'"';

    echo ' heats_scheduled="'.read_single_value('SELECT COUNT(*) FROM'
												.' (SELECT DISTINCT heat'
                                                .'   FROM RaceChart'
                                                .'   WHERE roundid = :roundid) AS tmp',
                                                array(':roundid' => $roundid)).'"';

    echo ' heats_run="'.read_single_value('SELECT COUNT(*) FROM'
										  .' (SELECT DISTINCT heat'
										  .'   FROM RaceChart'
										  .'   WHERE roundid = :roundid'
										  .'   AND finishtime IS NOT NULL) AS tmp',
										  array(':roundid' => $roundid)).'"';
    echo '/>'."\n";
}
?>
</coordinator_poll>
