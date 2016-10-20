<?php   // awardid
if (have_permission(EDIT_AWARDS_PERMISSION)) {
  start_response();
  try {
    $stmt = $db->prepare('DELETE FROM Awards WHERE awardid = :awardid');
    if ($stmt->execute(array(':awardid' => $_POST['awardid']))) {
      echo "<success/>\n";
      require('ajax/query.award.list.inc');
    } else {
      echo "<failure/>\n";
    }
  } catch (Exception $e) {
    echo "<failure code='sql'>".htmlspecialchars($e->getMessage(), ENT_QUOTES, 'UTF-8')."</failure>\n";
  }

  end_response();
} else {
  not_authorized();
}
?>