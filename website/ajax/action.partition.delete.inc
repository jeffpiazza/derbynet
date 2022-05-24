<?php

// $_POST['partitionid']

require_once('inc/partitions.inc');

$ok = true;
if (!have_permission(SET_UP_PERMISSION)) {
  json_not_authorized();
  $ok = false;
}

$partitionid = $_POST['partitionid'];
if ($ok && read_single_value('SELECT COUNT(*) FROM RegistrationInfo'
                             .' WHERE partitionid = :partitionid',
                             array(':partitionid' => $partitionid)) > 0) {
  json_failure('in-use',
               'The '.partition_label_lc().' cannot be deleted because it has racers assigned');
  $ok = false;
}

if ($ok) {
  list($classid, $rankid) = read_single_row('SELECT classid, rankid'
                                            .' FROM Partitions JOIN Ranks USING (rankid)'
                                            .' WHERE partitionid = :partitionid',
                                            array(':partitionid' => $partitionid));

  execute_once('DELETE FROM Partitions WHERE partitionid = :partitionid',
               array(':partitionid' => $partitionid));

  // if by-partition, delete the corresponding class, if there are no racing results
  // if one-group, delete the corresponding rank.
  switch (group_formation_rule()) {
  case 'one-group':
    // The rank is no longer associated to any partition, and so should be deleted.
    if (read_single_value('SELECT COUNT(*) FROM RegistrationInfo WHERE rankid = :rankid',
                          array(':rankid' => $rankid)) == 0) {
      execute_once('DELETE FROM Ranks WHERE rankid = :rankid',
                   array(':rankid' => $rankid));
    }
    break;
  case 'by-partition':
    if (read_single_value('SELECT COUNT(*) FROM RegistrationInfo WHERE classid = :classid',
                          array(':classid' => $classid)) == 0) {
      execute_once('DELETE FROM Ranks WHERE classid = :classid',
                   array(':classid' => $classid));
      execute_once('DELETE FROM RaceChart WHERE roundid IN'
                   .'  (SELECT roundid FROM Rounds'
                   .'   WHERE classid = :classid)',
                   array(':classid' => $classid));
      execute_once('DELETE FROM Roster WHERE roundid IN'
                   .'  (SELECT roundid FROM Rounds'
                   .'   WHERE classid = :classid)',
                   array(':classid' => $classid));
      execute_once('DELETE FROM Rounds WHERE classid = :classid',
                   array(':classid' => $classid));
      execute_once('DELETE FROM Playlist WHERE classid = :classid',
                   array(':classid' => $classid));
      execute_once('DELETE FROM ConstituentClasses WHERE antecedent = :classid',
                   array(':classid' => $classid));
      execute_once('DELETE FROM Classes WHERE classid = :classid',
                   array(':classid' => $classid));
    }
    break;
  }

  json_success();
}