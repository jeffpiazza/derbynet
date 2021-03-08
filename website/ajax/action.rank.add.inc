<?php
// Adding a rank:
//  $_POST['name']
//  $_POST['classid']

require_once('inc/schema_version.inc');
require_once('inc/sql-script.inc');
require_once('inc/classes-xml.inc');
require_once(sql_file_path('column-widths'));
require_once('inc/events.inc');

start_response();

if (have_permission(SET_UP_PERMISSION)) {
  if (isset($_POST['name']) && isset($_POST['classid'])) {
    $rankname = substr(trim($_POST['name']), 0, RANK_COLUMN_WIDTH);
    $classid = $_POST['classid'];
    if ($rankname == '') {
       echo "<failure code='noname'>No name provided for the new ".group_label()."</failure>\n";
    } else if (read_single_value('SELECT COUNT(*) FROM Classes WHERE classid = :classid',
                                 array(':classid' => $classid)) == 0) {
      echo "<failure code='noclass'>No class has classid ".$classid."</failure>\n";
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

      echo "<success/>\n";
      emit_all_classes();
    }
  } else {
    echo "<failure code='missing-args'>Unrecognized argument signature</failure>\n";
  }
} else {
  echo "<failure code='permission'>Not authorized</failure>\n";
}

end_response();
?>