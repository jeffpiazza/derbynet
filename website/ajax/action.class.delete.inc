<?php

// $_POST['classid'] -- the classid of the class to delete.
//
// Deletes all the ranks within the class, as well as all race results and scheduled rounds for the class.
require_once('inc/json-classes.inc');
require_once('inc/schema_version.inc');
require_once('inc/events.inc');

$classid = $_POST['classid'];

if (have_permission(SET_UP_PERMISSION)) {
  try {
    $ok = true;

    if ($ok) {
      $nracers = read_single_value('SELECT COUNT(*) FROM RegistrationInfo WHERE classid = :classid',
                                   array(':classid' => $classid));
      if ($nracers > 0) {
        json_failure('existing-racers', "Unable to delete a class with racers");
        $ok = false;
      }
    }
    if ($ok && schema_version() >= 3) {
      $stmt = $db->prepare('SELECT class, constituents FROM Classes');
      $stmt->execute(array());
      foreach ($stmt as $row) {
        if (in_array($classid, explode(',', $row['constituents']))) {
          json_failure('constituent', "Unable to delete a constituent class");
          $ok = false;
        }
      }
    }
    
    if ($ok) {
      record_event_delete_rank($classid, -1);
      $stmt = $db->prepare('DELETE FROM Ranks WHERE classid = :classid');
      if (!$stmt->execute(array(':classid' => $classid))) {
        json_failure('delete-ranks', "Unable to delete Ranks entries");
        $ok = false;
      }
    }
    if ($ok) {
      $stmt = $db->prepare('DELETE FROM RaceChart'
                           .' WHERE roundid IN'
                           .'  (SELECT roundid FROM Rounds'
                           .'   WHERE classid = :classid)');
      if (!$stmt->execute(array(':classid' => $classid))) {
        json_failure('delete-racechart', "Unable to delete RaceChart entries");
        $ok = false;
      }
      if ($stmt->rowCount() > 0) {
        record_event_delete_schedule(-1, "Deleting class $classid");
      }
    }
    if ($ok) {
      record_event_delete_round($classid, -1);
      $stmt = $db->prepare('DELETE FROM Rounds  WHERE classid = :classid');
      if (!$stmt->execute(array(':classid' => $classid))) {
        json_failure('delete-rounds', "Unable to delete Rounds entries");
        $ok = false;
      }
    }
    if ($ok) {
      $stmt = $db->prepare('DELETE FROM Playlist WHERE classid = :classid');
      if (!$stmt->execute(array(':classid' => $classid))) {
        json_failure('delete-rounds', "Unable to delete Playlist entries");
        $ok = false;
      }
    }
    if ($ok) {
      $stmt = $db->prepare('DELETE FROM Classes WHERE classid = :classid');
      if (!$stmt->execute(array(':classid' => $classid))) {
        json_failure('delete-classes', "Unable to delete Classes entries");
        $ok = false;
      }
      if ($stmt->rowCount() > 0) {
        record_event_delete_class($classid);
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