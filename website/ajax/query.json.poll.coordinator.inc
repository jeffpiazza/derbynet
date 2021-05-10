<?php // $_GET['roundid'] and $_GET['heat'] identify the "current" heat as the
 // coordinator page sees it.

require_once('inc/aggregate_round.inc');
require_once('inc/json-classes.inc');
require_once('inc/json-current-heat.inc');
require_once('inc/json-current-racers.inc');
require_once('inc/json-heat-results.inc');
require_once('inc/json-last-heat.inc');
require_once('inc/json-replay.inc');
require_once('inc/json-rounds.inc');
require_once('inc/json-timer-state.inc');
require_once('inc/name-mangler.inc');

$now_running = get_running_round();
json_out('current-heat', json_current_heat($now_running));
json_out('racers', json_current_racers($now_running, FULL_NAME, /* photos */ false));
json_out('timer-state', json_timer_state());
json_out('replay-state', json_replay());
json_out('last-heat', json_last_heat());

json_out('heat-results',
         json_heat_results(isset($_GET['roundid']) ? $_GET['roundid'] : '0',
                           isset($_GET['heat']) ? $_GET['heat'] : '1'));

json_out('classes', json_all_classes());
json_out('rounds', json_all_rounds());

// Pre-defined aggregate classes that could be scheduled for racing:
json_out('ready-aggregate', array());
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
    $json_out['ready-aggregate'][] = array(
      'classid' => $agg_class['classid'] + 0,
      'by-subgroup' => empty($agg_class['rankids']) ? false : true,
      'class' => $agg_class['class']);
  }
}


?>
