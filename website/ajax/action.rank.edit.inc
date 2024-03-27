<?php
// Renaming a rank:
// $_POST['rankid']
// $_POST['name']

require_once('inc/json-classes.inc');
require_once('inc/partitions.inc');
require_once(sql_file_path('column-widths'));

if (have_permission(SET_UP_PERMISSION)) {
  if (isset($_POST['rankid']) && isset($_POST['name'])) {
    $name = substr(trim($_POST['name']), 0, RANK_COLUMN_WIDTH);
    $stmt = $db->prepare('UPDATE Ranks SET rank = :name WHERE rankid = :rankid');
    $stmt->execute(array(':rankid' => $_POST['rankid'],
                         ':name' => $name));

    $stmt = $db->prepare('UPDATE Partitions SET name = :name WHERE rankid = :rankid');
    $stmt->execute(array(':rankid' => $_POST['rankid'],
                         ':name' => $name));

    json_out('classes', json_all_classes());
    json_success();
  } else {
    json_failure('missing-args', "Unrecognized argument signature");
  }
} else {
  json_not_authorized();
}
?>
