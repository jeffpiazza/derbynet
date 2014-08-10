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

  echo "<new-row>\n";
  $racer_id = read_single_value('SELECT racerid FROM RegistrationInfo'
                                .' WHERE firstname = :firstname'
                                .' AND lastname = :lastname'
                                .' AND rankid = :rankid',
                                array(':firstname' => $firstname,
                                      ':lastname' => $lastname,
                                      ':rankid' => $rankid));
  checkin_table_row($racer_id, 
                    htmlspecialchars($firstname, ENT_QUOTES, 'UTF-8'),
                    htmlspecialchars($lastname, ENT_QUOTES, 'UTF-8'),
                    $carno, $rankid, $classname, $rankname, 
                    /* passed */false, /* racer_xbs */false,
                    /*scheduled*/false, /*den_scheduled TODO don't know!*/false,
                    read_raceinfo_boolean('xbs-award'),
                    read_raceinfo_boolean('use-subgroups'));
  echo "\n</new-row>\n";
} else {
  not_authorized_body();
}

  end_response();
?>