<coordinator_poll>
<?php /* Current heat */
require('inc/current-heat.inc');
require('inc/current-racers.inc');
?>
<?php /* Timer */ 
require_once('inc/timer-state.inc');
$tstate = get_timer_state();
$tcontact = read_raceinfo('timer_last_contact');
if ($tstate == TIMER_NOT_CONNECTED) {
  $msg = 'NOT CONNECTED';
  $icon = 'img/status_not_connected.png';
} else if ($tstate == TIMER_CONNECTED) {
  $msg ='CONNECTED ('.(time() - $tcontact).' sec.)';
  $icon = 'img/status_ok.png';
} else if ($tstate == TIMER_PREPARED) {
  $msg = 'Prepared';
  $icon = 'img/status_ok.png';
} else if ($tstate == TIMER_RUNNING) {
  $msg = 'Race running';
  $icon = 'img/status_ok.png';
} else {
  $msg = 'Unknown ('.$tstate.')';
  $icon = 'img/status_trouble.png';
}
echo '<timer-state lanes="'.get_lane_count().'"'
    .' last_contact="'.$tcontact.'"'
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
  $icon = 'img/status_not_connected.png';
} else if ($replay['state'] == REPLAY_CONNECTED) {
  $msg = 'CONNECTED ('.(time() - $replay['last_contact']).' sec.)';
  $icon = 'img/status_ok.png';
} else {
  $msg = 'TROUBLE: '.$replay['trouble'];
  $icon = 'img/status_trouble.png';
}
echo '<replay-state host-and-port="'.$replay['host_and_port'].'"'
    .' last_contact="'.$replay['last_contact'].'"'
    .' state="'.$replay['state'].'"'
    .' icon="'.$icon.'"'
    .' connected="'.($replay['state'] != REPLAY_NOT_CONNECTED).'"'
    .'>';
echo $msg;
echo "</replay-state>\n";
?>
<?php // Kiosks
require_once('inc/kiosks.inc');
try {
  drop_old_kiosks();
  $stmt = $db->query('SELECT address, name, page, last_contact FROM Kiosks ORDER BY name, address, last_contact');
  foreach ($stmt as $row) {
    echo '<kiosk>'."\n";
    echo '  <name>'.htmlspecialchars($row['name'], ENT_QUOTES, 'UTF-8').'</name>'."\n";
    echo '  <address>'.htmlspecialchars($row['address'], ENT_QUOTES, 'UTF-8').'</address>'."\n";
    echo '  <last_contact>'.$row['last_contact'].'</last_contact>'."\n";
    echo '  <assigned_page>'.$row['page'].'</assigned_page>'."\n";
    echo '</kiosk>'."\n";
    }
} catch (PDOException $p) {
    // TODO: No Kiosks table
    echo '<h2>No Kiosks table defined.</h2>';  // TODO: Or other problem...
    // TODO: What if not throwing exceptions?
    // TODO: Create kiosk table on demand
    // TODO: Allow assigning name to kiosk
}

xml_for_kiosk_pages('kiosks');
xml_for_kiosk_pages('local'.DIRECTORY_SEPARATOR.'kiosks');

?>
<?php // Rounds

$curr_round = $now_running; // Avoid an extra DB call: get_running_round();

$stmt = $db->query('SELECT roundid, Classes.class, round FROM Rounds'
                   .' INNER JOIN Classes ON Rounds.classid = Classes.classid'
                   .' ORDER BY round, Classes.class');
$rounds = array();
foreach ($stmt as $round) {
    $rounds[] = $round;
}


foreach ($rounds as $round) {
    $roundid = $round['roundid'];

    echo '<round roundid="'.$roundid.'"'
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