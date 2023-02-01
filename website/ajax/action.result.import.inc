<?php

function new_sortorder_value($table) {
  return read_single_value("SELECT 1 + COALESCE(MAX(sortorder), 0) FROM $table");
}

// $read_fields and $write_fields are as would be passed to $stmt->execute(), but the
// keys, minus a colon, have to correspond with column names in the table.
function get_or_create_id($idfield, $tablename, $read_fields, $write_fields = null) {
  global $db;
  if (!$write_fields) {
    $write_fields = $read_fields;
  }
  $read_sql = 'SELECT '.$idfield.' FROM '.$tablename;
  $first = true;
  foreach ($read_fields as $param => $val) {
    $read_sql .= ($first ? ' WHERE ' : ' AND ');
    $read_sql .= substr($param, 1).' = '.$param;
    $first = false;
  }

  $id = read_single_value($read_sql, $read_fields);
  if (!$id) {
    $write_sql = 'INSERT INTO '.$tablename;
    $first = true;
    $write_params = '';
    foreach ($write_fields as $param => $val) {
      $write_sql .= ($first ? '(' : ', ').substr($param, 1);
      $write_params .= ($first ? '' : ', ').$param;
      $first = false;
    }
    $write_sql .= ') VALUES ('.$write_params.')';
    $stmt = $db->prepare($write_sql);
    if (!$stmt->execute($write_fields)) {
      json_sql_failure($write_sql);
    }

    $id = read_single_value($read_sql, $read_fields);
  }

  if (!$id) {
    json_failure('unable', "Unable to create entry in $tablename table.");
  }
  // TODO Column width?

  return $id;
}

if (have_permission(SET_UP_PERMISSION)) {
  $classid = get_or_create_id('classid', 'Classes',
                              array(':class' => $_POST['class']),
                              array(':class' => $_POST['class'],
                                    ':sortorder' => new_sortorder_value('Classes')));
  // Any rankid will do, but there needs to be at least one
  $rankid = get_or_create_id('rankid', 'Ranks',
                             array(':classid' => $classid),
                             array(':rank' => $_POST['class'],
                                   ':classid' => $classid,
                                   ':sortorder' => new_sortorder_value('Ranks')));
  $partitionid = get_or_create_id('partitionid', 'Partitions',
                                 array(':rankid' => $rankid),
                                 array(':name' => $_POST['class'],
                                       ':rankid' => $rankid,
                                       ':sortorder' => new_sortorder_value('Partitions')));
  $racerid = get_or_create_id('racerid', 'RegistrationInfo',
                              array(':lastname' => $_POST['lastname'],
                                    ':firstname' => $_POST['firstname'],
                                    ':carnumber' => $_POST['carnumber'],
                                    ':partitionid' => $partitionid,
                                    ':classid' => $classid),
                              array(':lastname' => $_POST['lastname'],
                                    ':firstname' => $_POST['firstname'],
                                    ':carnumber' => $_POST['carnumber'],
                                    ':carname' => $_POST['carname'],
                                    ':partitionid' => $partitionid,
                                    ':classid' => $classid,
                                    ':rankid' => $rankid,
                                    ':passedinspection' => 1,
                                    ':exclude' => 0));
  $roundid = get_or_create_id('roundid', 'Rounds',
                              array(':round' => $_POST['round'],
                                    ':classid' => $classid));

  // These are "get_or_create" so that import operations can be idempotent.
  get_or_create_id('resultid', 'RaceChart',
                   array(':classid' => $classid,
                         ':roundid' => $roundid,
                         ':heat' => $_POST['heat'],
                         ':lane' => $_POST['lane'],
                         ':racerid' => $racerid,
                         ':finishtime' => $_POST['finishtime'],
                         ':finishplace' => $_POST['finishplace'],
                         ':completed' => $_POST['completed']));

  get_or_create_id('rosterid', 'Roster',
                   array(':roundid' => $roundid,
                         ':classid' => $classid,
                         ':racerid' => $racerid));
  if ($json_out['outcome']['summary'] != 'failure') {
    json_success();
  }
} else {
  json_not_authorized();
}
?>
