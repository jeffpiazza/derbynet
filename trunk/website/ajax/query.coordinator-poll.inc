<coordinator_poll>
<?php /* Current heat */
require('inc/current-heat.inc');
require('inc/current-racers.inc');
?>
<?php /* Timer */ 
require_once('inc/timer-state.inc');
$tstate = get_timer_state();
$tcontact = read_raceinfo('timer_last_contact');
echo '<timer-state lanes="'.get_lane_count().'" last_contact="'.$tcontact.'">';
    if ($tstate == TIMER_NOT_CONNECTED) {
        echo 'NOT CONNECTED';
    } else if ($tstate == TIMER_CONNECTED) {
        echo 'CONNECTED ('.(time() - $tcontact).' sec.)';
    } else if ($tstate == TIMER_PREPARED) {
        echo 'Prepared';
    } else if ($tstate == TIMER_RUNNING) {
        echo 'Race running';
    } else {
        echo 'Unknown ('.$tstate.')';
    }
echo '</timer-state>'."\n";
?>
<?php /* Replay */
require_once('inc/replay.inc');
$replay = read_replay_host_and_port();
$rcontact = read_raceinfo('replay_last_contact');
echo '<replay-state host-and-port="'.$replay.'" last_contact="'.$rcontact.'">';
if ($replay == '') {
  echo 'NOT CONNECTED';
} else {
  echo 'CONNECTED at '.$replay.' ('.(time() - $rcontact).' sec.)';
}
echo "</replay-state>\n";
?>
<?php // Kiosks
require_once('inc/kiosks.inc');
try {
  drop_old_kiosks();
  $stmt = $db->query('SELECT address, name, page, last_contact FROM Kiosks ORDER BY name, address, last_contact');
  foreach ($stmt as $row) {
    echo '<kiosk>'."\n";
    echo '  <name>'.htmlspecialchars($row['name']).'</name>'."\n";
    echo '  <address>'.htmlspecialchars($row['address']).'</address>'."\n";
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
              .' class="'.htmlspecialchars($round['class'], ENT_QUOTES).'"'
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

    echo ' heats_scheduled="'.read_single_value('SELECT COUNT(distinct heat)'
                                                .' FROM RaceChart'
                                                .' WHERE roundid = :roundid',
                                                array(':roundid' => $roundid)).'"';

    echo ' heats_run="'.read_single_value('SELECT COUNT(distinct heat)'
                                   .' FROM RaceChart'
                                   .' WHERE roundid = :roundid'
                                   .' AND finishtime IS NOT NULL',
                                   array(':roundid' => $roundid)).'"';
    echo '/>'."\n";
}
?>
</coordinator_poll>
