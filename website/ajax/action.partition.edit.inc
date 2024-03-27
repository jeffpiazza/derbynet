<?php

// $_POST['partitionid']
// $_POST['name']

require_once('inc/partitions.inc');
require_once('inc/events.inc');

// Renaming a partition should rename the corresponding group (by-partition) or subgroup (one-group)

$ok = true;
if (!have_permission(SET_UP_PERMISSION)) {
  json_not_authorized();
  $ok = false;
}

$name = trim($_POST['name']);
$partitionid = $_POST['partitionid'];

if ($ok && (empty($name) || $name === '')) {
  json_failure('empty-name', 'Name must not be empty');
  $ok = false;
}

if ($ok && read_single_value('SELECT COUNT(*) FROM Partitions'
                             .' WHERE name = :name AND partitionid <> :partitionid',
                             array(':name' => $name,
                                   ':partitionid' => $partitionid)) != 0) {
  json_failure('name-taken', 'The name '.$name.' is already in use.');
  $ok = false;
}

if ($ok) {
  $old_name = read_single_value('SELECT name FROM Partitions WHERE partitionid = :partitionid',
                                array(':partitionid' => $partitionid), '(Not found)');
  record_event(EVENT_PARTITION_RENAMED, array('other' => $old_name.' to '.$name));

  execute_once('UPDATE Partitions SET name = :name WHERE partitionid = :partitionid',
               array(':name' => $name,
                     ':partitionid' => $partitionid));

  if (group_formation_rule() == 'by-partition') {
    execute_once('UPDATE Classes SET class = :name'
                 .' WHERE classid = (SELECT classid'
                 .'                  FROM '.inner_join('Partitions', 'Ranks',
                                                       'Partitions.rankid = Ranks.rankid')
                 .'                  WHERE partitionid = :partitionid)',
                 array(':name' => $name,
                       ':partitionid' => $partitionid));
  }

  execute_once('UPDATE Ranks SET rank = :name'
                 .' WHERE rankid = (SELECT rankid FROM Partitions'
                 .'                 WHERE partitionid = :partitionid)',
                 array(':name' => $name,
                       ':partitionid' => $partitionid));

  json_success();
}

?>
