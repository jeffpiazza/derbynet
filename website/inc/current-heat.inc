<?php
$now_running = get_running_round();
$use_master_sched = use_master_sched();
?><current-heat
    now-racing="<?php echo @$now_running['now_racing']; ?>"
    use-master-sched="<?php echo @$now_running['use_master_sched']; ?>"
    classid="<?php echo @$now_running['classid']; ?>"
    roundid="<?php echo @$now_running['roundid']; ?>"
    round="<?php echo @$now_running['round']; ?>"
    tbodyid="<?php echo $use_master_sched ? @$now_running['round'] : @$now_running['roundid']; ?>"
    heat="<?php echo @$now_running['heat']; ?>"
    number-of-heats="<?php echo read_single_value('SELECT MAX(heat) FROM RaceChart'
                                                  .' WHERE roundid = :roundid',
                                                  array(':roundid' => @$now_running['roundid']));
?>"><?php echo htmlspecialchars(@$now_running['class'], ENT_QUOTES, 'UTF-8'); ?></current-heat>
