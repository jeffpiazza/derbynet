<?php
require_once('inc/partitions.inc');
require_once('inc/schema_version.inc');
require_once('inc/sql-script.inc');
require_once(sql_file_path('column-widths'));
require_once('inc/events.inc');

function find_or_create_rank($rankname, $classid, $with_partition = true) {
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
    // Partition names are required to be unique, but rank names are unique only
    // within a class.  So it could happen that the natural partition name is
    // already taken, and associated with a different rank.

    // If find_or_create_partition ends up creating a partition, the recursive call
    // to find_or_create_rank should find the same $rankid.
    if ($with_partition) {
      $partid = find_or_create_partition($rankname);
      if (read_single_value('SELECT rankid FROM Partitions WHERE partitionid = :partitionid',
                            array(':partitionid' => $partid)) != $rankid) {
        $cl = read_single_value('SELECT class FROM Classes where classid ='
                                .' (SELECT classid FROM Ranks WHERE rankid = :rankid)',
                                array(':rankid' => $rankid));
        $suffix_n = 0;
        $suffix = '';
        while (read_single_value('SELECT COUNT(*) FROM Partitions WHERE name = :pname',
                                 array(':pname' => $cl.'-'.$rankname.$suffix)) > 0) {
          $suffix_n = 1 + $suffix_n;
          $suffix = '-'.$suffix_n;
        }
        create_partition_in_existing_rank($cl.'-'.$rankname.$suffix, $rankid);
      }
    }

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
      record_action(array('action' => 'rank.*create',
                          'rank' => $rankname,
                          'rankid' => $rankid,
                          'classid' => $classid));
    }
  }

  return $rankid;
}

function delete_rank($rankid) {
  global $db;
  // Assumes that there are no racers in this rank.
  $rankname = read_single_value('SELECT rank FROM Ranks WHERE rankid = :rankid',
                                array(':rankid' => $rankid), '(No rank name)');
  record_event(EVENT_RANK_DELETED, array('classid' => -1,
                                         'rankid' => $rankid,
                                         'other' => $rankname));
  record_action(array('action' => 'rank.*delete', 'rankid' => $rankid));

  $stmt = $db->prepare('DELETE FROM Ranks WHERE rankid = :rankid');
  $stmt->execute(array(':rankid' => $rankid));

  $stmt = $db->prepare('DELETE FROM Awards WHERE rankid = :rankid');
  $stmt->execute(array(':rankid' => $rankid));

  // If there are no racers in the rank, there shouldn't be any in any of the
  // partitions, so safe to delete these.
  $stmt = $db->prepare('DELETE FROM Partitions WHERE rankid = :rankid');
  $stmt->execute(array(':rankid' => $rankid));
}
?>
