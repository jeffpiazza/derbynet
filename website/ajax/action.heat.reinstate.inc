<?php

require_once('inc/last-heat.inc');
require_once('inc/write-heat-results.inc');
require_once('inc/racing-state.inc');
require_once('inc/events.inc');
require_once('inc/event-lane-details.inc');

if (!have_permission(CONTROL_RACE_PERMISSION)) {
  json_not_authorized();
} else {
  $ok = true;
  $last_heat = get_last_heat();
  $roundid = array_shift($last_heat);
  $heat = array_shift($last_heat);

  record_event_for_reinstated_heat($roundid, $heat);

  $result_column = read_raceinfo_boolean('use-points') ? 'finishplace' : 'finishtime';
  $lane = 1;
  foreach ($last_heat as $result) {
    take_action_silently('UPDATE RaceChart'
                         .' SET '.$result_column.' = :result,'
                         .' completed = '.dbnow_expr()
                         .' WHERE roundid = :roundid'
                         .'  AND heat = :heat'
                         .'  AND lane = :lane',
                         array(':result' => $result,
                               ':roundid' => $roundid,
                               ':heat' => $heat,
                               ':lane' => $lane));
    $lane = $lane + 1;
  }
  clear_last_heat();
  set_racing_state(0);

  json_success();
  require('ajax/query.poll.coordinator.inc');
}
?>
