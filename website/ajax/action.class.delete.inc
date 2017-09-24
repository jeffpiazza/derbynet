<?php

if (have_permission(SET_UP_PERMISSION)) {
  start_response();

  // Mysterious classid fields:
  //
  // RaceChart has a classid ?
  // Roster

  try {
    $ok = true;
    if ($ok) {
      $stmt = $db->prepare('DELETE FROM Ranks WHERE classid = :classid');
      if (!$stmt->execute(array(':classid' => $_POST['classid']))) {
        echo "\n<failure code='delete-ranks'>Unable to delete Ranks entries</failure>\n";
        $ok = false;
      }
    }
    if ($ok) {
      $stmt = $db->prepare('DELETE FROM RaceChart'
                           .' WHERE roundid IN'
                           .'  (SELECT roundid FROM Rounds'
                           .'   WHERE classid = :classid)');
      if (!$stmt->execute(array(':classid' => $_POST['classid']))) {
        echo "\n<failure code='delete-racechart'>Unable to delete RaceChart entries</failure>\n";
        $ok = false;
      }
    }
    if ($ok) {
      $stmt = $db->prepare('DELETE FROM Rounds  WHERE classid = :classid');
      if (!$stmt->execute(array(':classid' => $_POST['classid']))) {
        echo "\n<failure code='delete-rounds'>Unable to delete Rounds entries</failure>\n";
        $ok = false;
      }
    }
    if ($ok) {
      $stmt = $db->prepare('DELETE FROM Classes WHERE classid = :classid');
      if (!$stmt->execute(array(':classid' => $_POST['classid']))) {
        echo "\n<failure code='delete-classes'>Unable to delete Classes entries</failure>\n";
        $ok = false;
      }
    }
    if ($ok) {
      echo "<success/>\n";
      require('ajax/query.class.list.inc');
    }
  } catch (Exception $e) {
    echo "<failure code='sql'>".htmlspecialchars($e->getMessage(), ENT_QUOTES, 'UTF-8')."</failure>\n";
  }

  end_response();
} else {
  not_authorized();
}
?>