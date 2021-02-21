<?php

// $_POST['classid'] -- the classid of the class to delete.
//
// Deletes all the ranks within the class, as well as all race results and scheduled rounds for the class.
require_once('inc/classes-xml.inc');
require_once('inc/schema_version.inc');

$classid = $_POST['classid'];

if (have_permission(SET_UP_PERMISSION)) {
  start_response();

  try {
    $ok = true;

    if ($ok) {
      $nracers = read_single_value('SELECT COUNT(*) FROM RegistrationInfo WHERE classid = :classid',
                                   array(':classid' => $classid));
      if ($nracers > 0) {
        echo "\n<failure code='existing-racers'>Unable to delete a class with racers</failure>\n";
        $ok = false;
      }
    }
    if ($ok && schema_version() >= 3) {
      $stmt = $db->prepare('SELECT class, constituents FROM Classes');
      $stmt->execute(array());
      foreach ($stmt as $row) {
        if (in_array($classid, explode(',', $row['constituents']))) {
          echo "\n<failure code='constituent'>Unable to delete a constituent class</failure>\n";
          $ok = false;
        }
      }
    }
    
    if ($ok) {
      $stmt = $db->prepare('DELETE FROM Ranks WHERE classid = :classid');
      if (!$stmt->execute(array(':classid' => $classid))) {
        echo "\n<failure code='delete-ranks'>Unable to delete Ranks entries</failure>\n";
        $ok = false;
      }
    }
    if ($ok) {
      $stmt = $db->prepare('DELETE FROM RaceChart'
                           .' WHERE roundid IN'
                           .'  (SELECT roundid FROM Rounds'
                           .'   WHERE classid = :classid)');
      if (!$stmt->execute(array(':classid' => $classid))) {
        echo "\n<failure code='delete-racechart'>Unable to delete RaceChart entries</failure>\n";
        $ok = false;
      }
    }
    if ($ok) {
      $stmt = $db->prepare('DELETE FROM Rounds  WHERE classid = :classid');
      if (!$stmt->execute(array(':classid' => $classid))) {
        echo "\n<failure code='delete-rounds'>Unable to delete Rounds entries</failure>\n";
        $ok = false;
      }
    }
    if ($ok) {
      $stmt = $db->prepare('DELETE FROM Playlist WHERE classid = :classid');
      if (!$stmt->execute(array(':classid' => $classid))) {
        echo "\n<failure code='delete-rounds'>Unable to delete Playlist entries</failure>\n";
        $ok = false;
      }
    }
    if ($ok) {
      $stmt = $db->prepare('DELETE FROM Classes WHERE classid = :classid');
      if (!$stmt->execute(array(':classid' => $classid))) {
        echo "\n<failure code='delete-classes'>Unable to delete Classes entries</failure>\n";
        $ok = false;
      }
    }
    if ($ok) {
      echo "<success/>\n";
      emit_all_classes();
    }
  } catch (Exception $e) {
    echo "<failure code='sql'>".htmlspecialchars($e->getMessage(), ENT_QUOTES, 'UTF-8')."</failure>\n";
  }

  end_response();
} else {
  not_authorized();
}
?>