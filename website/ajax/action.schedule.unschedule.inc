<?php
// $_POST: roundid

$roundid = $_POST['roundid'];

require_once('inc/autoadvance.inc');
require_once('inc/data.inc');
require_once('inc/events.inc');
require_once('inc/masterheat.inc');
require_once('inc/racing-state.inc');

$now_running = get_running_round();  // now-racing, classid, roundid, round

if (have_permission(CONTROL_RACE_PERMISSION)) {
  record_event(EVENT_SCHEDULE_DELETED, array('roundid' => $roundid, 'other' => ''));
  $stmt = $db->prepare('DELETE FROM RaceChart WHERE roundid = :roundid');
  $stmt->execute(array(':roundid' => $roundid));

  assign_master_heats();
  if ($roundid == $now_running['roundid']) {
    set_racing_state(0);
    set_current_heat(-1, -1);
    write_raceinfo('ClassID', -1);
  }
  if (use_master_sched()) {
    $current = get_running_round();
    advance_heat($current, 'next-up', /* report failure */ false);
  }
  
  json_success();
  require('ajax/query.poll.coordinator.inc');
} else {
  json_not_authorized();
}
?>
