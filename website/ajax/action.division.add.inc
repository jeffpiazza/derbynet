<?php

// $_POST['name']

require_once('inc/divisions.inc');

$ok = true;

if ($ok && !have_permission(CONTROL_RACE_PERMISSION)) {
  json_not_authorized();
  $ok = false;
}

$name = trim($_POST['name']);

if ($ok && (empty($name) || $name === '')) {
  json_failure('no-name', 'Division name may not be empty');
  $ok = false;
}

if ($ok && read_single_value('SELECT divisionid FROM Divisions WHERE name = :name',
                             array(':name' => $name)) !== false) {
  json_failure('already-exists', 'A division named '.$name.' already exists.');
  $ok = false;
}

if ($ok) {
  find_or_create_division($name);
  json_success();
}

?>