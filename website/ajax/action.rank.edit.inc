<?php
// Renaming a rank:
// $_POST['rankid']
// $_POST['name']

require_once('inc/json-classes.inc');

if (have_permission(SET_UP_PERMISSION)) {
  if (isset($_POST['rankid']) && isset($_POST['name'])) {
    $stmt = $db->prepare('UPDATE Ranks SET rank = :name WHERE rankid = :rankid');
    $stmt->execute(array(':rankid' => $_POST['rankid'],
                         ':name' => trim($_POST['name'])));
    json_out('classes', json_all_classes());
    json_success();
  } else {
    json_failure('missing-args', "Unrecognized argument signature");
  }
} else {
  json_not_authorized();
}
?>