<?php

require_once('inc/newracer.inc');
require_once('inc/checkin-table.inc');

start_response();

if (have_permission(REGISTER_NEW_RACER_PERMISSION)) {
  $firstname = $_POST['firstname'];
  $lastname = $_POST['lastname'];
  $rankid = $_POST['rankid'];

  if ($rankid == "") {
    $rankid = force_populate_a_rankid();
    echo "<reload/>";
  }

  insert_new_racer(array('firstname' => $firstname,
                         'lastname' => $lastname,
                         'carnumber' => $_POST['carno'],
                         'carname' => $_POST['carname'],
                         'rankid' => $rankid,
                         'exclude' => @$_POST['exclude'] ? true : false));

  echo "<success/>\n";

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