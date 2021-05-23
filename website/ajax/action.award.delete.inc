<?php   // awardid
require_once('inc/json-awards.inc');
require_once('inc/json-classes.inc');

if (have_permission(EDIT_AWARDS_PERMISSION)) {
  try {
    $stmt = $db->prepare('DELETE FROM Awards WHERE awardid = :awardid');
    if ($stmt->execute(array(':awardid' => $_POST['awardid']))) {
      json_success();
      json_out_awards(false);
    } else {
      json_failure('unknown', 'unknown');
    }
  } catch (Exception $e) {
    json_failure('sql', $e->getMessage());
  }
} else {
  json_not_authorized();
}
?>