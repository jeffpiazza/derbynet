<?php

if (have_permission(EDIT_RACER_PERMISSION)) {
  $racerid = $_POST['racer'];
  $rankid = $_POST['rankid'];
  $classid = read_single_value('SELECT classid FROM Ranks WHERE rankid = :rankid',
                               array(':rankid' => $rankid));
  if (!$classid) {
    echo '<checkin><failure>No class for rankid '.$rankid.'</failure></checkin>';
    exit();
  }

  $assignments = 'SET rankid = :rankid,'
  .' classid = :classid';
  $values = array(':racerid' => $racerid,
                  ':rankid' => $rankid,
                  ':classid' => $classid);
  if (isset($_POST['firstname'])) {
    $assignments .= ', firstname = :firstname';
    $values[':firstname'] = trim($_POST['firstname']);
  }
  if (isset($_POST['lastname'])) {
    $assignments .= ', lastname = :lastname';
    $values[':lastname'] = trim($_POST['lastname']);
  }
  if (isset($_POST['carno'])) {
    $assignments .= ', carnumber = :carnumber';
    $values[':carnumber'] = trim($_POST['carno']);
  }
  if (isset($_POST['carname'])) {
    $assignments .= ', carname = :carname';
    $values[':carname'] = trim($_POST['carname']);
  }
  if (isset($_POST['exclude'])) {
    $assignments .= ', exclude = :exclude';
    $values[':exclude'] = $_POST['exclude'];
  }

  $new_roundid = read_single_value('SELECT roundid FROM Rounds WHERE round = 1 AND classid = :classid',
                                   array(':classid' => $classid));

  $old_classid = read_single_value('SELECT classid FROM RegistrationInfo WHERE racerid = :racerid',
                                   array(':racerid' => $racerid));
  $old_roundid = read_single_value('SELECT roundid FROM Rounds WHERE round = 1 AND classid = :classid',
                                   array(':classid' => $old_classid));

  if (take_action_silently('UPDATE RegistrationInfo '.$assignments.' WHERE racerid = :racerid', $values)) {
    take_action('UPDATE Roster'
                .' SET roundid = :new_roundid,'
                .'     classid = :classid'
                .' WHERE racerid = :racerid'
                .' AND roundid = :old_roundid',
                array(':new_roundid' => $new_roundid,
                      ':classid' => $classid,
                      ':racerid' => $racerid,
                      ':old_roundid' => $old_roundid));
  }
} else {
  not_authorized();
}

?>