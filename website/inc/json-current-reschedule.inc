<?php

require_once('inc/rounds.inc');

function json_current_reschedule(&$now_running) {
  $roundid = $now_running['roundid'];
  $unsched = read_single_value('SELECT COUNT(*)'.sql_for_unscheduled(':roundid'),
                               array(':roundid' => $roundid));
  $departed = read_single_value('SELECT COUNT(DISTINCT racerid) '.sql_for_departed(':roundid'),
                                array(':roundid' => $roundid));
  return ($unsched > 0) || ($departed > 0);
}

?>
