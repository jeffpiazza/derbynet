<?php

$ok = true;

if (have_permission(EDIT_RACER_PERMISSION)) {
  if (isset($_POST['racer'])) {
    $racerid = $_POST['racer'];
  } else if (isset($_POST['racerid'])) {
    $racerid = $_POST['racerid'];
  } else {
    json_failure('noracer', "No racerid provided.");
    $ok = false;
  }

  $assignments = 'SET racerid = :racerid_again';
  $values = array(':racerid' => $racerid,
                  ':racerid_again' => $racerid);

  if ($ok && isset($_POST['partitionid'])) {
    $partition_id = $_POST['partitionid'];
    json_out('partition-sortorder',
             read_single_value('SELECT sortorder FROM Partitions'
                               .' WHERE partitionid = :partitionid',
                               array(':partitionid' => $partition_id)));
    $row = read_single_row('SELECT classid, rankid FROM Ranks'
                           .' WHERE rankid = (SELECT rankid FROM Partitions'
                           .'                 WHERE partitionid = :partitionid)',
                           array(':partitionid' => $partition_id), PDO::FETCH_ASSOC);
    if ($row) {
      $assignments .= ', partitionid = :partitionid';
      $values[':partitionid'] = $partition_id;

      $assignments .= ', classid = :classid, rankid = :rankid';
      $values[':classid'] = $row['classid'];
      $values[':rankid'] = $row['rankid'];
    } else {
      json_failure('nopartition', "Invalid segment number $partition_id");
      $ok = false;
    }
  }
  if ($ok && isset($_POST['rankid'])) {
    $rankid = $_POST['rankid'];
    $classid = read_single_value('SELECT classid FROM Ranks WHERE rankid = :rankid',
                                 array(':rankid' => $rankid));
    if (!$classid) {
      json_failure('noclass', "No class for rankid $rankid");
      $ok = false;
    }

    $assignments .= ', rankid = :rankid';
    $values[':rankid'] = $rankid;
    $assignments .= ', classid = :classid';
    $values[':classid'] = $classid;
  }
  if (isset($_POST['firstname'])) {
    $assignments .= ', firstname = :firstname';
    $values[':firstname'] = trim($_POST['firstname']);
  }
  if (isset($_POST['lastname'])) {
    $assignments .= ', lastname = :lastname';
    $values[':lastname'] = trim($_POST['lastname']);
  }
  if ($ok && isset($_POST['carno'])) {
    $assignments .= ', carnumber = :carnumber';
    $values[':carnumber'] = trim($_POST['carno']);
    if (read_single_value('SELECT COUNT(*) FROM RegistrationInfo'
                          .' WHERE carnumber = :carnumber'
                          .' AND racerid <> :racerid',
                          array(':carnumber' => $values[':carnumber'],
                                ':racerid' => $racerid)) > 0) {
      json_out('warning', array("Duplicate car number $values[carnumber]"));
    }
  }
  if (isset($_POST['carname'])) {
    $assignments .= ', carname = :carname';
    $values[':carname'] = trim($_POST['carname']);
  }
  if (isset($_POST['note_from'])) {
    $assignments .= ', note = :note_from';
    $values[':note_from'] = trim($_POST['note_from']);
  }
  if (isset($_POST['exclude'])) {
    $assignments .= ', exclude = :exclude';
    $values[':exclude'] = $_POST['exclude'];
  }

  if ($ok) {
    $old_classid = read_single_value('SELECT classid FROM RegistrationInfo WHERE racerid = :racerid',
                                     array(':racerid' => $racerid));
    $old_roundid = read_single_value('SELECT roundid FROM Rounds WHERE round = 1 AND classid = :classid',
                                     array(':classid' => $old_classid));

    $ok = take_action_silently('UPDATE RegistrationInfo '.$assignments
                               .' WHERE racerid = :racerid', $values);
    if (!$ok) {
      json_failure('sql', "$info[0]-$info[1] $sql failed: $info[2]");
    }
  }

  if ($ok && isset($values[':classid'])) {
    $new_roundid = read_single_value('SELECT roundid FROM Rounds WHERE round = 1 AND classid = :classid',
                                     array(':classid' => $values[':classid']));

    $ok = take_action_silently('UPDATE Roster'
                               .' SET roundid = :new_roundid,'
                               .'     classid = :classid'
                               .' WHERE racerid = :racerid'
                               .' AND roundid = :old_roundid',
                               array(':new_roundid' => $new_roundid,
                                     ':classid' => $values[':classid'],
                                     ':racerid' => $racerid,
                                     ':old_roundid' => $old_roundid));

    if (!$ok) {
      json_failure('sql', "$info[0]-$info[1] $sql failed: $info[2]");
    }
  }

  if ($ok) {
    json_success();
  }
} else {
  json_not_authorized();
}
?>
