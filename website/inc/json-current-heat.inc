<?php

function json_current_heat(&$now_running) {
  $current_heat = array(
    'now_racing' => $now_running['now_racing'] ? true : false,
    'use_master_sched' => @$now_running['use_master_sched'] ? true : false,
    'use_points' => @$now_running['use_points'] ? true : false,
    'classid' => @$now_running['classid'],
    'roundid' => @$now_running['roundid'],
    'round' => @$now_running['round'],
    'tbodyid' => ($now_running['use_master_sched'] ? @$now_running['round'] : @$now_running['roundid']),
    'heat' => @$now_running['heat'],
    'number-of-heats' => read_single_value('SELECT MAX(heat) FROM RaceChart'
                                           .' WHERE roundid = :roundid',
                                           array(':roundid' => @$now_running['roundid']), 0));
  if (use_groups()) {
    $current_heat['class'] = @$now_running['class'];
  }
  if (@$now_running['use_master_sched']) {
    $current_heat['masterheat'] = read_single_value('SELECT masterheat FROM RaceChart'
                                                    .' WHERE roundid = :roundid'
                                                    .'   AND heat = :heat',
                                                    array(':roundid' => $now_running['roundid'],
                                                          ':heat' => $now_running['heat']));
    $current_heat['max_masterheat'] = read_single_value('SELECT MAX(masterheat) FROM RaceChart');
  }
  return $current_heat;
}

?>
