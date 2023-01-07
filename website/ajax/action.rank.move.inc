<?php
// Move a rank (subgroup) from class (group) to another
// $_POST['rankid']
// $_POST['classid'], or -1 to create a new group from this subgroup.

require_once('inc/class_add_or_delete.inc');
require_once('inc/partitions.inc');

if (have_permission(SET_UP_PERMISSION)) {
  $rankid = $_POST['rankid'];
  $classid = $_POST['classid'];
  if ($classid < 0) {
    $rankname = read_single_value('SELECT rank FROM Ranks WHERE rankid = :rankid',
                                  array(':rankid' => $rankid));
    $classid = create_class(choose_unused_class_name($rankname));
  }
  $stmt = $db->prepare('UPDATE Ranks SET classid = :classid,'
                       .' sortorder = 1 + (SELECT COUNT(*) FROM Ranks WHERE classid = :classid2)'
                       .' WHERE rankid = :rankid');
  $stmt->execute(array(':classid' => $classid,
                       ':classid2' => $classid,
                       ':rankid' => $rankid));
  $stmt = $db->prepare('UPDATE RegistrationInfo SET classid = :classid'
                       .' WHERE rankid = :rankid');
  $stmt->execute(array(':classid' => $classid,
                       ':rankid' => $rankid));
  $stmt = $db->prepare('UPDATE Roster'
                       .' SET roundid = (SELECT roundid FROM Rounds'
                       .'                WHERE classid = :classid'
                       .'                  AND round = 1),'
                       .'     classid = :classid2'
                       .' WHERE racerid IN '
                       .'  (SELECT racerid FROM RegistrationInfo WHERE rankid = :rankid)'
                       .' AND roundid IN (SELECT roundid FROM Rounds'
                       .'                 WHERE round = 1)');
  $stmt->execute(array(':classid' => $classid,
                       ':classid2' => $classid,
                       ':rankid' => $rankid));

  clean_up_empty_classes_and_ranks();

  json_success();
} else {
  json_failure('permission', "Not authorized");
}
?>
