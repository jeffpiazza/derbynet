<?php

start_response();

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
      echo "<failure code=\"sql-write\">SQL execute failed: ".$write_sql."</failure>\n";
      echo "<error-info>\n"; var_dump($db->errorInfo()); echo "</error-info>\n";
    }

    $id = read_single_value($read_sql, $read_fields);
  }

  if (!$id) {
    echo '<failure code="unable">Unable to create entry in '.$tablename.' table.</failure>'."\n\n";
  }
  // TODO Column width?

  return $id;
}

if (have_permission(SET_UP_PERMISSION)) {
  $classid = get_or_create_id('classid', 'Classes',
                              array(':class' => $_POST['class']));
  // Any rankid will do, but there needs to be at least one
  $rankid = get_or_create_id('rankid', 'Ranks',
                             array(':classid' => $classid),
                             array(':rank' => $_POST['class'],
                                   ':classid' => $classid));
  $racerid = get_or_create_id('racerid', 'RegistrationInfo',
                              array(':lastname' => $_POST['lastname'],
                                    ':firstname' => $_POST['firstname'],
                                    ':carnumber' => $_POST['carnumber']),
                              array(':carnumber' => $_POST['carnumber'],
                                    ':lastname' => $_POST['lastname'],
                                    ':firstname' => $_POST['firstname'],
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
  echo "<success/>\n";
} else {
  not_authorized_body();
}

end_response();
?>