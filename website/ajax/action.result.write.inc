<?php
// NOTE: Timer results are handled in action.timer-message.inc.  This
// script is for manually-entered results and test purposes.

// $_POST['lane1'], $_POST['lane2'], etc. for times
// Optionally, $_POST['place1'], $_POST['place2'], etc., for places (by lane)

require_once('inc/write-heat-results.inc');
require_once('inc/autoadvance.inc');
require_once('inc/racing-state.inc');
require_once('inc/events.inc');

start_response();

if (have_permission(CONTROL_RACE_PERMISSION)) {
  // Capture events for all re-written results
  $current = get_running_round();
  $all_resultids = array();
  $completed = '';  // We expect only one distinct 'completed' value
  $stmt = $db->prepare('SELECT resultid, completed FROM RaceChart'
                       .' WHERE roundid = :roundid AND heat = :heat'
                       .' AND completed IS NOT NULL AND completed != \'\''
                       .' AND finishtime IS NOT NULL AND finishplace IS NOT NULL'
                       .' ORDER BY lane');
  $stmt->execute(array(':roundid' => $current['roundid'],
                       ':heat' => $current['heat']));
  foreach ($stmt as $resultid) {
    $all_resultids[] = $resultid['resultid'];
    $completed = $resultid['completed'];
  }
  if ($completed) {
    record_event_sql('INSERT INTO Events(tstamp, action, roundid, heat)'
                     .' VALUES(:tstamp, :action, :roundid, :heat)',
                     array(':tstamp' => $completed,
                           ':action' => EVENT_HEAT_COMPLETED,
                           ':roundid' => $current['roundid'],
                           ':heat' => $current['heat']));
  }

  foreach ($all_resultids as $resultid) {
    record_event_for_discarded_result($resultid);
  }

  if (write_heat_results(parse_heat_results(get_lane_count()))) {
    record_event_sql('INSERT INTO Events(action, roundid, heat) VALUES(:action, :roundid, :heat)',
                     array(':action' => EVENT_HEAT_MANUALLY_ENTERED,
                           ':roundid' => $current['roundid'],
                           ':heat' => $current['heat']));
    if (get_racing_state()) {
      $current = get_running_round();
      advance_heat($current, 'next', /* show failure */ false);
    }
    echo '<success/>';
    require('ajax/query.poll.coordinator.inc');
  }
} else {
  not_authorized_body();
}

end_response();
