<coordinator_poll>
<?php
require_once('inc/current-heat.inc');
require_once('inc/current-racers.inc');
require_once('inc/schema_version.inc');
require_once('inc/aggregate_round.inc');
require_once('inc/remote-start.inc');
require_once('inc/rounds.inc');
require_once('inc/last-heat.inc');
require_once('inc/timer-state-xml.inc');

$now_running = get_running_round();
emit_current_heat($now_running);
emit_current_racers($now_running, FULL_NAME, /* photos */ false);
emit_timer_state();
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
<?php  // last_heat
echo last_heat_xml_element()."\n";
?>
<?php // Rounds

$curr_round = $now_running; // Avoid an extra DB call: get_running_round();

$rounds = all_rounds_with_counts();

foreach ($rounds as $round) {
    $roundid = $round['roundid'];

    echo '<round roundid="'.$roundid.'"'
            .' classid="'.$round['classid'].'"'
            .' class="'.htmlspecialchars($round['class'], ENT_QUOTES, 'UTF-8').'"'
            .' round="'.$round['round'].'"';
    echo " aggregate=\"$round[aggregate]\"";
    echo " roster_size=\"$round[roster_size]\"";
    echo " passed=\"$round[passed]\"";
    echo " unscheduled=\"$round[unscheduled]\"";
    echo " heats_scheduled=\"$round[heats_scheduled]\"";
    echo " heats_run=\"$round[heats_run]\"";
    echo '>'.htmlspecialchars($round['roundname'], ENT_QUOTES, 'UTF-8')."</round>\n";
}

// Pre-defined aggregate classes that could be scheduled for racing:
foreach (aggregate_classes() as $agg_class) {
  $ready_to_race =
      read_single_value('SELECT COUNT(*) FROM Rounds WHERE classid = :classid',
                        array(':classid' => $agg_class['classid'])) == 0;
  if ($ready_to_race) {
    foreach ($agg_class['constituents'] as $classid) {
      if (read_single_value('SELECT COUNT(*) FROM RaceChart'
                            .' JOIN Rounds'
                            .' ON RaceChart.roundid = Rounds.roundid'
                            .' WHERE Rounds.classid = :classid'
                            .'  AND (completed IS NOT NULL AND completed <> \'\')',
                            array(':classid' => $classid))
          == 0) {
        $ready_to_race = false;
      }
    }
  }
  if ($ready_to_race) {
    echo "<ready-aggregate classid=\"$agg_class[classid]\">";
    echo htmlspecialchars($agg_class['class'], ENT_QUOTES, 'UTF-8');
    echo "</ready-aggregate>\n";
  }
}
?>
</coordinator_poll>
