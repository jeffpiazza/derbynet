<?php

// $_POST['rule']: 'one-group' or 'by-partition' (or 'custom')

require_once('inc/events.inc');
require_once('inc/form_groups_by_rule.inc');
require_once('inc/newracer.inc');
require_once('inc/partitions.inc');


if (!have_permission(CONTROL_RACE_PERMISSION)) {
  json_not_authorized();
} else if (!is_allowed_group_formation_rule($_POST['rule'])) {
  json_failure('bad-rule', 'Rule value '.$_POST['rule'].' not recognized.');
} else if (read_single_value('SELECT COUNT(*) FROM RaceChart'
                             .' WHERE finishtime IS NOT NULL OR finishplace IS NOT NULL')
           > 0) {
  json_failure('results-exist', 'Racing results already exist.');
} else {
  record_event(EVENT_GROUP_FORMATION_RULE_APPLIED,
               array('other' => group_formation_rule().' -> '.$_POST['rule']));

  write_group_formation_rule($_POST['rule']);

  form_groups_by_rule($_POST['rule']);

  clean_up_empty_classes_and_ranks();

  record_action_partition_structure();

  write_raceinfo('use-subgroups', ($_POST['rule'] == 'by-partition') ? 0 : 1);

  json_success();
}

?>
