<?php
// action.php?query=poll.now-racing&roundid= &heat= row-height=
//  Passed-in roundid & heat affect what heat-results, if any, are returned.
//    (That's the only thing they affect, though.)
//  row-height is used for constructing photo URLs, if any, so photos will be
//    the right size.
//  repo overrides the now-racing repo
//
// Returns:
//    current-heat: {now-racing: use_master_sched: use_points:
//                   classid roundid round tbodyid heat number-of-heats}
//  [ timer-trouble: true   if there's a problem with the timer ]
//    heat-results: [{lane, time, place, speed}...] if results are available
//    racers: [{lane name carname carnumber photo}...]

require_once('inc/json-current-heat.inc');
require_once('inc/json-current-racers.inc');
require_once('inc/json-heat-results.inc');
require_once('inc/photos-on-now-racing.inc');
require_once('inc/replay.inc');
require_once('inc/timer-state.inc');
require_once('inc/timer-test.inc');

$roundid = isset($_GET['roundid']) ? $_GET['roundid'] : '0';
$heat = isset($_GET['heat']) ? $_GET['heat'] : '1';

if (warn_no_timer()) {
  $timer_state = new TimerState();
  if ($timer_state->troubled()) {
    json_out('timer-trouble', true);
  }
}

$now_running = get_running_round();
json_out('current-heat', json_current_heat($now_running));

// This lets the js reserve room for the right number of digits
$time_format = get_finishtime_formatting_string();
json_out('precision', $time_format == "%6.4f" ? 4 : 3);

if ($now_running['roundid'] == TIMER_TEST_ROUNDID) {
  json_out('timer-test', true);
  if ($roundid == TIMER_TEST_ROUNDID) {
    $tt = timer_test_recorded_results();
    json_out('tt', $tt);
    json_out('heat-results', $tt['results']);
  }

  $racers = array();
  $nlanes = get_lane_count();
  for ($lane = 1; $lane <= $nlanes; ++$lane) {
    if (($tt['mask'] & (1 << ($lane - 1))) == 0) {
      $racers[] = array('lane' => $lane);
    }
  }
  json_out('racers', $racers);
} else {
  json_out('heat-results', json_heat_results($roundid, $heat));

  $render = false;
  if (isset($_GET['row-height']) && $_GET['row-height'] > 0) {
    if (isset($_GET['repo'])) {
      $photos = $_GET['repo'];
      if (!photo_repository($photos)) {
        $photos = 0;
      }
    } else {
      $photos = read_photos_on_now_racing();
    }
    if ($photos) {
      $render_name = (2 * $_GET['row-height']).'x'.$_GET['row-height'];
      $render = photo_repository($photos)->lookup_or_any($render_name);
    }
  }

  json_out('racers', json_current_racers($now_running, read_name_style(), $render));
}
?>

