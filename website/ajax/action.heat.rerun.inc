<?php

// $_POST['heat'] : 'current' or 'last'

require_once('inc/racing-state.inc');
require_once('inc/autoadvance.inc');
require_once('inc/events.inc');
require_once('inc/timer-state.inc');
require_once('inc/last-heat.inc');

start_response();

if (!have_permission(CONTROL_RACE_PERMISSION)) {
  not_authorized_body();
} else {
  $ok = true;

  if (@$_POST['heat'] == 'current') {
    $current = get_running_round();
    $roundid = $current['roundid'];
    $heat = $current['heat'];
  } else {
    // Rerunning last heat
    @list($roundid, $heat) = get_last_heat();
  }

  // Check that the heat we're trying to set as current actually exists
  $ok = read_single_value('SELECT COUNT(*) FROM RaceChart'
                          .' WHERE roundid = :roundid AND heat = :heat',
                          array(':roundid' => $roundid,
                                ':heat' => $heat)) > 0;
  if ($ok) {
    set_racing_state(0);

    send_replay_CANCEL();

    write_raceinfo('RoundID', $roundid);
    write_raceinfo('Heat', $heat);
  
    $current = get_running_round();

    send_replay_START($current);

    capture_last_heat_results($roundid, $heat);

    $resultid = read_single_value('SELECT resultid FROM RaceChart'
                                  .' WHERE roundid = :roundid'
                                  .' AND heat = :heat',
                                  array(':roundid' => $roundid,
                                        ':heat' => $heat));
    record_event_for_discarded_result($resultid);

    $stmt = $db->prepare('UPDATE RaceChart'
                         .' SET finishtime = NULL, finishplace = NULL, completed = NULL'
                         .' WHERE roundid = :roundid'
                         .' AND heat = :heat');
    $stmt->execute(array(':roundid' => $roundid,
                         ':heat' => $heat));

    if (set_racing_state(1)) {
      // If a heat was already staging (or running!), force it back to a CONNECTED
      // state, so that the next message from the timer will trigger a new
      // heat-ready element.  (No <abort/> gets sent, though.)
      $timer_state = new TimerState();
      if ($timer_state->state() == TIMER_STAGING || $timer_state->state() == TIMER_RUNNING) {
        $timer_state->to_state(TIMER_CONNECTED);
        $timer_state->write(false);
      }
    }
  } else {
    echo "<failure code='nosuchheat'>No such heat</failure>\n";
  }

  if ($ok) {
    echo "<success/>\n";
  }
}

require('ajax/query.poll.coordinator.inc');

end_response();
?>