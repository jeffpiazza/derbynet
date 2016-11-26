<?php
function emit_current_heat(&$now_running, $use_master_sched) {
  echo '<current-heat'
    .' now-racing="'.@$now_running['now_racing'].'"'
    .' use-master-sched="'.@$now_running['use_master_sched'].'"'
    .' classid="'.@$now_running['classid'].'"'
    .' roundid="'.@$now_running['roundid'].'"'
    .' round="'.@$now_running['round'].'"'
    .' tbodyid="'.($use_master_sched ? @$now_running['round'] : @$now_running['roundid']).'"'
    .' heat="'.@$now_running['heat'].'"'
    .' number-of-heats="'.read_single_value('SELECT MAX(heat) FROM RaceChart'
                                            .' WHERE roundid = :roundid',
                                            array(':roundid' => @$now_running['roundid'])).'"'
    .'>';
  echo htmlspecialchars(@$now_running['class'], ENT_QUOTES, 'UTF-8');
  echo "</current-heat>\n";
}
?>
