<?php
// Action to schedule heats for a round, with each car running once in each lane.
// (An exception is if max-runs-per-car is non-zero; see below.)
//

$roundid = $_POST['roundid'];
$n_times_per_lane = isset($_POST['n_times_per_lane']) ? $_POST['n_times_per_lane'] : 1;

$trace = !isset($_POST['trace']) ? false : array('trace');

//
// Normally we use max-runs-per-car = 0, so that each car runs once per lane.
// For some unusual situations, we want to limit to just 1 run per car (no
// matter which lane).  In that case, we first pad out the roster with BYEs so
// each car can get one run before we would start running anyone's second run.
// Then we form the rough schedule as a simple rotation chart (because the
// normal chart generators may put a given car only in heats that include a
// second run for some other car).  Finally, in make_ordered_schedule, we
// abandon those heats from the rough schedule that would cause any car to have
// second run.

require_once('inc/data.inc');
require_once('inc/masterheat.inc');
require_once('inc/autoadvance.inc');
require_once('inc/schedule_utils.inc');
require_once('inc/schedule_one_round.inc');

if (have_permission(CONTROL_RACE_PERMISSION)) {
  // $nlanes reports the number of physical lanes, even if some won't be used.
  $nlanes = read_raceinfo('lane_count', 0);
  // For generating the schedule, we care about the number of lanes actually in use.
  $nlanes -= count_one_bits(read_raceinfo('unused-lane-mask', 0));
  if ($nlanes <= 0) {
    json_failure('nlanes', 'Number of lanes hasn\'t been determined.');
// TODO
//  } else if (!isset($all_generators[$nlanes])) {
//    json_failure('too-many-lanes', 'Lane scheduling is only available for up to '
//    .max(array_keys($all_generators)).' lanes.');
  } else if (results_exist($roundid)) {
    json_failure('results-exist', 'Race results already exist for this round');
  } else {
	if (schedule_one_round($roundid, $nlanes, $n_times_per_lane, $trace)) {
      json_success();
      if ($trace) {
        json_out('trace', $trace);
      }
      require('ajax/query.poll.coordinator.inc');
    } else {
      json_failure('too-few-racers', 'There need to be at least two racers checked in'
                   .' to generate a schedule.');
    }
  }

  assign_master_heats();
  if (use_master_sched()) {
    $current = get_running_round();
    advance_heat($current, 'next-up', /* report failure */ false);
  }
} else {
  json_not_authorized();
}

?>
