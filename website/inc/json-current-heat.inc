<?php

function json_current_heat(&$now_running) {
  $current_heat = array(
    'now_racing' => $now_running['now_racing'] ? true : false,
    'use_master_sched' => @$now_running['use_master_sched'] ? true : false,
    'use_points' => @$now_running['use_points'] ? true : false,
    'classid' => @$now_running['classid'] + 0,
    'roundid' => @$now_running['roundid'] + 0,
    'round' => @$now_running['round'] + 0,
    'tbodyid' => ($use_master_sched ? @$now_running['round'] : @$now_running['roundid']) + 0,
    'heat' => @$now_running['heat'] + 0,
    'number-of-heats' => read_single_value('SELECT MAX(heat) FROM RaceChart'
                                           .' WHERE roundid = :roundid',
                                           array(':roundid' => @$now_running['roundid']), 0) + 0);
  if (use_groups()) {
    $current_heat['class'] = @$now_running['class'];
  }
  return $current_heat;
}

?>