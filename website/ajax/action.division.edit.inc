<?php

// $_POST['divisionid']
// $_POST['name']

require_once('inc/divisions.inc');

// Renaming a division should rename the corresponding group (by-division) or subgroup (one-group)

$ok = true;
if (!have_permission(SET_UP_PERMISSION)) {
  json_not_authorized();
  $ok = false;
}

$name = trim($_POST['name']);
$divisionid = $_POST['divisionid'];

if ($ok && (empty($name) || $name === '')) {
  json_failure('empty-name', 'Name must not be empty');
  $ok = false;
}

if ($ok && read_single_value('SELECT COUNT(*) FROM Divisions'
                             .' WHERE name = :name AND divisionid <> :divisionid',
                             array(':name' => $name,
                                   ':divisionid' => $divisionid)) != 0) {
  json_failure('name-taken', 'The name '.$name.' is already in use.');
  $ok = false;
}

if ($ok) {
  read_single_value('UPDATE Divisions SET name = :name WHERE divisionid = :divisionid',
                    array(':name' => $name,
                          ':divisionid' => $divisionid));

  switch (group_formation_rule()) {
  case 'by-division':
    read_single_value('UPDATE Classes SET class = :name'
                      .' WHERE classid = (SELECT classid'
                      .'                  FROM '.inner_join('Divisions', 'Ranks',
                                                            'Divisions.rankid = Ranks.rankid')
                      .'                  WHERE divisionid = :divisionid)',
                      array(':name' => $name,
                            ':divisionid' => $divisionid));
    break;
  case 'one-group':
    read_single_value('UPDATE Ranks SET rank = :name'
                      .' WHERE rankid = (SELECT rankid FROM Divisions'
                      .'                 WHERE divisionid = :divisionid)',
                      array(':name' => $name,
                            ':divisionid' => $divisionid));
    break;
  }

  json_success();
}

?>