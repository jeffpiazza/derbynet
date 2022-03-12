<?php

// $_POST['name']

require_once('inc/partitions.inc');

$ok = true;

if ($ok && !have_permission(CONTROL_RACE_PERMISSION)) {
  json_not_authorized();
  $ok = false;
}

$name = trim($_POST['name']);

if ($ok && (empty($name) || $name === '')) {
  json_failure('no-name', 'A '.partition_label_lc().' must have a name.');
  $ok = false;
}

if ($ok && read_single_value('SELECT partitionid FROM Partitions WHERE name = :name',
                             array(':name' => $name)) !== false) {
  json_failure('already-exists', 'A '.partition_label_lc().' named '.$name.' already exists.');
  $ok = false;
}

if ($ok) {
  $divid = find_or_create_partition($name);
  json_out('partitionid', $divid);
  json_success();
}

?>