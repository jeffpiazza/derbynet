<?php

// $_POST['div_id'] = the numeric divisionid
// $_POST['group_field'] = 'classid' or 'rankid'
// $_POST['group_id'] = numeric classid or rankid the (sub)division racers will move to.
//
//  // If the racers to move should be further limited to a particular class or rank, these are present:
// $_POST['from_group_field'] = 'classid' or 'rankid' (or empty if this is not used)
// $_POST['from_group_id']


require_once('inc/divisions.inc');
require_once('inc/newracer.inc');

function move_division($div_id, $group_field, $group_id, $from_group_field, $from_group_id) {
  global $db;

  if ($group_field == 'classid') {
    $name = read_single_value("SELECT name FROM Divisions WHERE divisionid = :div_id",
                              array(':div_id' => $div_id));
    if ($group_id < 0) {
      $group_id = find_or_create_class($name);
    }

    // Make a new subgroup for this division
    $classid = $group_id;
    $rankid = find_or_create_rank($name, $group_id);
  } else {
    if ($group_id < 0) {
      $group_id = find_or_create_rank($name, -$group_id);
    }
    $rankid = $group_id;
    $classid = read_single_value("SELECT classid FROM Ranks WHERE rankid = :group_id",
                                 array(':group_id' => $group_id));
  }
  
  $sql = "UPDATE RegistrationInfo SET classid = :classid, rankid = :rankid WHERE divisionid = :div_id";
  $params = array(':classid' => $classid,
                  ':rankid' => $rankid,
                  ':div_id' => $div_id);

  if ($from_group_field) {
    $sql .= " AND $from_group_field = :from_group_id";
    $params[':from_group_id'] = $from_group_id;
  }

  $stmt = $db->prepare($sql);
  $stmt->execute($params);
}


if (!have_permission(CONTROL_RACE_PERMISSION)) {
  json_not_authorized();
} else {
  move_division($_POST['div_id'],
                $_POST['group_field'], $_POST['group_id'],
                @$_POST['from_group_field'], @$_POST['from_group_id']);

  if (isset($_POST['cleanup']) && $_POST['cleanup']) {
    clean_up_empty_classes_and_ranks();
  }

  reorder_divisions();

  json_success();
}
?>