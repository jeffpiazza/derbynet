<?php

// $_POST['rankid'] -- the rankid of the rank to delete.
//
// Won't allow deleting the last rank within the class (delete the class instead).
// Won't allow deleting a rank with racers.
require_once('inc/json-classes.inc');
require_once('inc/events.inc');

$rankid = $_POST['rankid'];

if (have_permission(SET_UP_PERMISSION)) {
  try {
    $ok = true;

    if ($ok) {
      $nranks = read_single_value('SELECT COUNT(*) FROM Ranks'
                                  .' WHERE classid = (SELECT classid FROM Ranks'
                                  .'                  WHERE rankid = :rankid)',
                                  array(':rankid' => $rankid));
      if ($nranks < 2) {
        json_failure('last-rank', "Can't delete the last rank in a class");
        $ok = false;
      }
    }
    if ($ok) {
      $nracers = read_single_value('SELECT COUNT(*) FROM RegistrationInfo WHERE rankid = :rankid',
                                   array(':rankid' => $rankid));
      if ($nracers > 0) {
        json_failure('existing-racers', "Unable to delete a rank with racers");
        $ok = false;
      }
    }

    if ($ok) {
      record_event(EVENT_RANK_DELETED, array('classid' => -1,
                                             'rankid' => $rankid));
      $stmt = $db->prepare('DELETE FROM Ranks WHERE rankid = :rankid');
      if (!$stmt->execute(array(':rankid' => $rankid))) {
        json_failure('delete-ranks', "Unable to delete Ranks entries");
        $ok = false;
      }
    }
    if ($ok) {
      json_out('classes', json_all_classes());
      json_success();
    }
  } catch (Exception $e) {
    json_failure('sql', $e->getMessage());
  }
} else {
  json_not_authorized();
}
?>