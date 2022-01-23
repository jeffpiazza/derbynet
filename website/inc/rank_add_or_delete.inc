<?php
require_once('inc/schema_version.inc');
require_once('inc/sql-script.inc');
require_once(sql_file_path('column-widths'));
require_once('inc/events.inc');

function find_or_create_rank($rankname, $classid) {
  global $db;
  $rankname = substr(trim($rankname), 0, RANK_COLUMN_WIDTH);
  $rankid = read_single_value('SELECT rankid FROM Ranks'
                              .' WHERE rank = :rankname AND classid = :classid',
                              array(':rankname' => $rankname,
                                    ':classid' => $classid));
  if ($rankid === false) {
    $stmt = $db->prepare('INSERT INTO Ranks(rank, classid) VALUES(:rankname, :classid)');
    $stmt->execute(array(':rankname' => $rankname,
                         ':classid' => $classid));
    $rankid = read_single_value('SELECT rankid FROM Ranks'
                                .' WHERE rank = :rankname AND classid = :classid',
                                array(':rankname' => $rankname,
                                      ':classid' => $classid));
    if (schema_version() >= 2) {
      record_event(EVENT_RANK_ADDED, array('classid' => $classid,
                                           'rankid' => $rankid,
                                           'other' => $rankname));
      $sortorder = read_single_value('SELECT 1 + MAX(sortorder) FROM Ranks WHERE classid = :classid',
                                     array(':classid' => $classid));
      if ($sortorder === false) {
        $sortorder = 1;
      }
      $stmt = $db->prepare('UPDATE Ranks'
                           .' SET sortorder = :sortorder'
                           .' WHERE rankid = :rankid');
      $stmt->execute(array(':sortorder' => $sortorder,
                           ':rankid' => $rankid));
    }
  }

  return $rankid;
}
?>