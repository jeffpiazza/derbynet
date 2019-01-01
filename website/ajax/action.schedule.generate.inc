<?php
// Action to schedule heats for a round, with each car running once in each
// lane.  (An exception is if max-runs-per-car is non-zero; see comment in
// schedule_generate.inc.)
//
// $_POST: roundid, nrounds(=1)
// nrounds here is the number of times each racer should race in each lane.

$roundid = $_POST['roundid'];
$nrounds = isset($_POST['nrounds']) ? $_POST['nrounds'] : 1;

require_once('inc/masterheat.inc');
require_once('inc/schedule_generate.inc');
require_once('inc/schedule_utils.inc');

start_response();

if (have_permission(CONTROL_RACE_PERMISSION)) {
  // $nlanes reports the number of physical lanes, even if some won't be used.
  $nlanes = read_raceinfo('lane_count', 0);
  // For generating the schedule, we care about the number of lanes actually in use.
  $nlanes -= count_one_bits(read_raceinfo('unused-lane-mask', 0));
  if ($nlanes <= 0) {
	echo '<failure code="nlanes">Number of lanes hasn\'t been determined.</failure>';
  } else if (!isset($all_generators[$nlanes])) {
    echo '<failure code="too-many-lanes">Lane scheduling is only available for up to '
    .max(array_keys($all_generators)).' lanes.</failure>';
  } else if (results_exist($roundid)) {
	echo '<failure code="results-exist">Race results already exist for this round</failure>';
  } else {
	if (schedule_one_round($roundid, $nlanes, $nrounds)) {
      echo '<success/>';
      require('ajax/query.poll.coordinator.inc');
    } else {
      echo '<failure code="too-few-racers">There need to be at least two racers checked in'
      .' to generate a schedule.</failure>';
    }
  }

  assign_master_heats(round_number_from_roundid($roundid));
} else {
  not_authorized_body();
}

end_response();
?>
