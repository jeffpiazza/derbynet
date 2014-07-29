<coordinator_poll>
<?php require('inc/current-heat.inc'); ?>
<?php

$curr_round = get_running_round();
// TODO: Control of ordering of rounds

// TODO: Create 2nd round, grand finals round, including roster

$stmt = $db->query('SELECT roundid, Classes.class, round FROM Rounds'
                   .' INNER JOIN Classes ON Rounds.classid = Classes.classid'
                   .' ORDER BY round, Classes.class');
$rounds = array();
foreach ($stmt as $round) {
    $rounds[] = $round;
}



foreach ($rounds as $round) {
    $roundid = $round['roundid'];

/* roster size,
   n_passed,
   n_unscheduled // Number of passed racers not in schedule
   n_heats_scheduled
   n_heats_run */

    // TODO: htmlspecialchars for class
    echo '<round roundid="'.$roundid.'" class="'.$round['class'].'" round="'.$round['round'].'"';

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
