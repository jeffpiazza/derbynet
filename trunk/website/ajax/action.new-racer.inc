<?php

require_once('inc/newracer.inc');
require_once('inc/checkin-table.inc');

start_response();

if (have_permission(REGISTER_NEW_RACER_PERMISSION)) {
  $firstname = $_POST['firstname'];
  $lastname = $_POST['lastname'];
  $carno = $_POST['carno'];
  $rankid = $_POST['rankid'];
  $exclude = @$_POST['exclude'] ? true : false;

  insert_new_racer_rankid($firstname, $lastname, $carno, $rankid, $exclude);

  echo "<success/>\n";

  $stmt = $db->prepare('SELECT rank, classid FROM Ranks WHERE rankid = :rankid');
  $stmt->execute(array(':rankid' => $rankid));
  $result = $stmt->fetch();
  $rankname = $result[0];
  $classid = $result[1];
  $classname = read_single_value('SELECT class FROM Classes WHERE classid = :classid',
                                 array(':classid' => $classid));

  $racer_id = read_single_value('SELECT racerid FROM RegistrationInfo'
                                .' WHERE firstname = :firstname'
                                .' AND lastname = :lastname'
                                .' AND rankid = :rankid',
                                array(':firstname' => $firstname,
                                      ':lastname' => $lastname,
                                      ':rankid' => $rankid));
  echo "<new-row>\n";
  checkin_table_row_for_id($racer_id);
  echo "\n</new-row>\n";
} else {
  not_authorized_body();
}

  end_response();
?>