<?php   // awardid
require_once('inc/awards-xml.inc');

if (have_permission(EDIT_AWARDS_PERMISSION)) {
  start_response();
  try {
    $stmt = $db->prepare('DELETE FROM Awards WHERE awardid = :awardid');
    if ($stmt->execute(array(':awardid' => $_POST['awardid']))) {
      echo "<success/>\n";
      emit_award_list(/* include_ad_hoc */ false);
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