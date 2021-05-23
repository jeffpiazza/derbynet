<?php
// Adding a rank:
//  $_POST['name']
//  $_POST['classid']

require_once('inc/schema_version.inc');
require_once('inc/sql-script.inc');
require_once('inc/json-classes.inc');
require_once(sql_file_path('column-widths'));
require_once('inc/events.inc');

if (have_permission(SET_UP_PERMISSION)) {
  if (isset($_POST['name']) && isset($_POST['classid'])) {
    $rankname = substr(trim($_POST['name']), 0, RANK_COLUMN_WIDTH);
    $classid = $_POST['classid'];
    if ($rankname == '') {
      json_failure('noname', "No name provided for the new ".group_label());
    } else if (read_single_value('SELECT COUNT(*) FROM Classes WHERE classid = :classid',
                                 array(':classid' => $classid)) == 0) {
      json_failure('noclass', "No class has classid $classid");
    } else {
      $stmt = $db->prepare('INSERT INTO Ranks(rank, classid) VALUES(:name, :classid)');
      $stmt->execute(array(':name' => $rankname,
                           ':classid' => $classid));

      $rankid = read_single_value('SELECT rankid FROM Ranks'
                                  .' WHERE rank = :rankname AND classid = :classid',
                                  array(':rankname' => $rankname,
                                        ':classid' => $classid));
      if (schema_version() >= 2) {
        record_event_create_rank($classid, $rankid, $rankname);
        $nranks = read_single_value('SELECT COUNT(*) FROM Ranks WHERE classid = :classid',
                                    array(':classid' => $classid));
        $stmt = $db->prepare('UPDATE Ranks'
                             .' SET sortorder = :nranks'
                             .' WHERE rankid = :rankid');
        $stmt->execute(array(':nranks' => $nranks,
                             ':rankid' => $rankid));
      }

      json_out('classes', json_all_classes());
      json_success();
    }
  } else {
    json_failure('missing-args', "Unrecognized argument signature");
  }
} else {
  json_failure('permission', "Not authorized");
}
?>