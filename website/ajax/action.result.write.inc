<?php
// NOTE: Timer results are handled in action.timer-message.inc.  This
// script is for manually-entered results and test purposes.

// $_POST['lane1'], $_POST['lane2'], etc. for times
// Optionally, $_POST['place1'], $_POST['place2'], etc., for places (by lane)

require_once('inc/write-heat-results.inc');
require_once('inc/autoadvance.inc');
require_once('inc/racing-state.inc');
require_once('inc/events.inc');
require_once('inc/event-lane-details.inc');

if (have_permission(CONTROL_RACE_PERMISSION)) {
  $current = get_running_round();
  record_event_for_discarded_heat($current['roundid'], $current['heat']);
  $results = parse_heat_results(get_lane_count());
  if (write_heat_results($results)) {
    record_timer_finish(EVENT_HEAT_MANUALLY_ENTERED, $results, $current);
    $current = get_running_round();
    if (get_racing_state()) {
      advance_heat($current, 'next', /* show failure */ false);
    }
    json_success();
    require('ajax/query.poll.coordinator.inc');
  }
} else {
  json_not_authorized();
}
