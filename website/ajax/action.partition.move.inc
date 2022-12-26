<?php

// $_POST['div_id'] = the numeric partitionid
// $_POST['group_field'] = 'classid' or 'rankid'
// $_POST['group_id'] = numeric classid or rankid the (sub)partition racers will move to.
//
// If the racers to move should be further limited based on their
// currently-assigned class or rank, these are present:
//
// $_POST['from_group_field'] = 'classid' or 'rankid' (or empty if this is not used)
// $_POST['from_group_id']


require_once('inc/partitions.inc');
require_once('inc/newracer.inc');
require_once('inc/class_add_or_delete.inc');
require_once('inc/rank_add_or_delete.inc');
require_once('inc/events.inc');

function move_partition($div_id, $group_field, $group_id, $from_group_field, $from_group_id) {
  global $db;

  $name = read_single_value("SELECT name FROM Partitions WHERE partitionid = :div_id",
                            array(':div_id' => $div_id));

  if ($group_field == 'classid') {
    if ($group_id < 0) {
      $group_id = find_or_create_class($name);
    }

    // Make a new subgroup for this partition
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

  $classname = read_single_value('SELECT class FROM Classes WHERE classid = :classid',
                                 array(':classid' => $classid), '(No such group)');
  $rankname = read_single_value('SELECT rank FROM Ranks WHERE rankid = :rankid',
                                array(':rankid' => $rankid), '(No such subgroup)');
  record_event(EVENT_PARTITION_MOVED,
                 array('other' => $name.' to group '.$classname.', subgroup '.$rankname));

  
  // Delete roster entries for affected racers.  After moving the partition,
  // fill_in_missing_roster_entries() will create new roster entries.  (This
  // ignores from_group_field and from_group_id, so may needlessly delete and
  // restore some entries.)
  $stmt = $db->prepare("DELETE FROM Roster"
                       ." WHERE racerid IN (SELECT racerid FROM RegistrationInfo"
                       ."                   WHERE partitionid = :div_id)");
  $stmt->execute(array(':div_id' => $div_id));

  $stmt = $db->prepare('UPDATE Partitions SET rankid = :rankid WHERE partitionid = :partitionid');
  $stmt->execute(array(':rankid' => $rankid,
                       ':partitionid' => $div_id));
  
  $sql = "UPDATE RegistrationInfo SET classid = :classid, rankid = :rankid WHERE partitionid = :div_id";
  $params = array(':classid' => $classid,
                  ':rankid' => $rankid,
                  ':div_id' => $div_id);

  if ($from_group_field) {
    $sql .= " AND $from_group_field = :from_group_id";
    $params[':from_group_id'] = $from_group_id;
  }

  $stmt = $db->prepare($sql);
  $stmt->execute($params);

  fill_in_missing_roster_entries();
}


if (!have_permission(CONTROL_RACE_PERMISSION)) {
  json_not_authorized();
} else {
  move_partition($_POST['div_id'],
                $_POST['group_field'], $_POST['group_id'],
                @$_POST['from_group_field'], @$_POST['from_group_id']);

  clean_up_empty_classes_and_ranks();

  reorder_partitions();

  json_success();
}
?>
