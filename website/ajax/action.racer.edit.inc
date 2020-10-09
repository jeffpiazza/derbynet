<?php

start_response();

if (have_permission(EDIT_RACER_PERMISSION)) {
  if (isset($_POST['racer'])) {
    $racerid = $_POST['racer'];
  } else if (isset($_POST['racerid'])) {
    $racerid = $_POST['racerid'];
  } else {
    echo "<failure code='noracer'>No racerid provided.</failure>\n";
    end_response();
    exit();
  }

  $assignments = 'SET racerid = :racerid_again';
  $values = array(':racerid' => $racerid,
                  ':racerid_again' => $racerid);

  if (isset($_POST['rankid'])) {
    $rankid = $_POST['rankid'];
    $classid = read_single_value('SELECT classid FROM Ranks WHERE rankid = :rankid',
                                 array(':rankid' => $rankid));
    if (!$classid) {
      echo '<failure>No class for rankid '.$rankid.'</checkin>';
      end_response();
      exit();
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
  if (isset($_POST['carno'])) {
    $assignments .= ', carnumber = :carnumber';
    $values[':carnumber'] = trim($_POST['carno']);
    if (read_single_value('SELECT COUNT(*) FROM RegistrationInfo'
                          .' WHERE carnumber = :carnumber'
                          .' AND racerid <> :racerid',
                          array(':carnumber' => $values[':carnumber'],
                                ':racerid' => $racerid)) > 0) {
      echo "<warning>Duplicate car number ".$values[':carnumber']."</warning>\n";
    }
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

  $ok = true;
  $ok = take_action_silently('UPDATE RegistrationInfo '.$assignments
                             .' WHERE racerid = :racerid', $values);
  if ($ok && isset($classid)) {
    $ok = take_action_silently('UPDATE Roster'
                               .' SET roundid = :new_roundid,'
                               .'     classid = :classid'
                               .' WHERE racerid = :racerid'
                               .' AND roundid = :old_roundid',
                               array(':new_roundid' => $new_roundid,
                                     ':classid' => $classid,
                                     ':racerid' => $racerid,
                                     ':old_roundid' => $old_roundid));
  }

  if ($ok) {
    echo'<success/>'.$body_xml;
  } else {
    $info = $db->errorInfo();
    echo '<failure code="sql'.$info[0].'-'.$info[1].'">'
    .$sql.' failed: '.$info[2].' [EOM]</failure>';
  }
} else {
  not_authorized_body();
}

end_response();
?>