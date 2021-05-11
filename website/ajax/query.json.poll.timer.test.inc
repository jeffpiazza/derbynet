<?php
require_once('inc/json-current-heat.inc');
require_once('inc/json-timer-state.inc');
require_once('inc/timer-test.inc');

$now_running = get_running_round();
json_out('current-heat', json_current_heat($now_running));
json_out('timer-state', json_timer_state());
json_out('timer-details', json_timer_details());
json_out('tt', timer_test_recorded_results());
?>
