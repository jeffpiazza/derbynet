<?php

if (have_permission(EDIT_RACER_PERMISSION)) {
  $racerid = $_POST['racer'];
  $firstname = $_POST['firstname'];
  $lastname = $_POST['lastname'];
  $carno = $_POST['carno'];
  $carname = $_POST['carname'];
  $rankid = $_POST['rankid'];
  $classid = read_single_value('SELECT classid FROM Ranks WHERE rankid = :rankid',
                               array(':rankid' => $rankid));
  if (!$classid) {
    echo '<checkin><failure>['.$fetch.']</failure></checkin>';
    exit();
  }
  $exclude = $_POST['exclude'];

  $new_roundid = read_single_value('SELECT roundid FROM Rounds WHERE round = 1 AND classid = :classid',
                                   array(':classid' => $classid));
  $old_classid = read_single_value('SELECT classid FROM RegistrationInfo WHERE racerid = :racerid',
                                   array(':racerid' => $racerid));
  $old_roundid = read_single_value('SELECT roundid FROM Rounds WHERE round = 1 AND classid = :classid',
                                   array(':classid' => $old_classid));

  if (take_action_silently('UPDATE RegistrationInfo'
                           .' SET firstname = :firstname,'
                           .' lastname = :lastname,'
                           .' carnumber = :carnumber,'
                           .' carname = :carname,'
                           .' rankid = :rankid,'
                           .' classid = :classid,'
                           .' exclude = :exclude'
                           .' WHERE racerid = :racerid',
                           array(':firstname' => $firstname,
                                 ':lastname' => $lastname,
                                 ':carnumber' => $carno,
                                 ':carname' => $carname,
                                 ':rankid' => $rankid,
                                 ':classid' => $classid,
                                 ':exclude' => $exclude,
                                 ':racerid' => $racerid))) {
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