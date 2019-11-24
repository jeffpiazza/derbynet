<?php

// $_POST['rankid'] -- the rankid of the rank to delete.
//
// Won't allow deleting the last rank within the class (delete the class instead).
// Won't allow deleting a rank with racers.
require_once('inc/classes-xml.inc');

$rankid = $_POST['rankid'];

if (have_permission(SET_UP_PERMISSION)) {
  start_response();

  try {
    $ok = true;

    if ($ok) {
      $nranks = read_single_value('SELECT COUNT(*) FROM Ranks'
                                  .' WHERE classid = (SELECT classid FROM Ranks'
                                  .'                  WHERE rankid = :rankid)',
                                  array(':rankid' => $rankid));
      if ($nranks < 2) {
        echo "\n<failure code='last-rank'>Can't delete the last rank in a class</failure>\n";
        $ok = false;
      }
    }
    if ($ok) {
      $nracers = read_single_value('SELECT COUNT(*) FROM RegistrationInfo WHERE rankid = :rankid',
                                   array(':rankid' => $rankid));
      if ($nracers > 0) {
        echo "\n<failure code='existing-racers'>Unable to delete a rank with racers</failure>\n";
        $ok = false;
      }
    }

    if ($ok) {
      $stmt = $db->prepare('DELETE FROM Ranks WHERE rankid = :rankid');
      if (!$stmt->execute(array(':rankid' => $rankid))) {
        echo "\n<failure code='delete-ranks'>Unable to delete Ranks entries</failure>\n";
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