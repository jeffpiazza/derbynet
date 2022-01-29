<?php
// Renaming a class:
// $_POST['classid']
// $_POST['name']
// $_POST['ntrophies']

require_once('inc/classes.inc');
require_once('inc/partitions.inc');

if (have_permission(SET_UP_PERMISSION)) {
  if (isset($_POST['classid']) && isset($_POST['name'])) {
    $name = trim($_POST['name']);
    $stmt = $db->prepare('UPDATE Classes SET class = :name WHERE classid = :classid');
    $stmt->execute(array(':classid' => $_POST['classid'],
                         ':name' => $name));
    if (group_formation_rule() == 'by-partition') {
      // In 'by-partition', the group, the subgroup, and the partition all
      // represent the same set, so rename all to match.
      $stmt = $db->prepare('UPDATE Ranks SET rank = :name WHERE classid = :classid');
      $stmt->execute(array(':classid' => $_POST['classid'],
                           ':name' => $name));
      $stmt = $db->prepare('UPDATE Partitions SET name = :name'
                           .' WHERE rankid IN (SELECT rankid FROM Ranks'
                           .'                  WHERE classid = :classid)');
      $stmt->execute(array(':classid' => $_POST['classid'],
                           ':name' => $name));
    }
    if (isset($_POST['ntrophies'])) {
      update_ntrophies($_POST['classid'], $_POST['ntrophies']);
    }
    json_success();
  } else {
    json_failure('missing-args', "Unrecognized argument signature");
  }
} else {
  json_failure('permission', "Not authorized");
}
?>