<?php

require_once('inc/newracer.inc');
require_once('inc/checkin-table.inc');

if (have_permission(REGISTER_NEW_RACER_PERMISSION)) {
  $firstname = $_POST['firstname'];
  $lastname = $_POST['lastname'];
  $partitionid = $_POST['partitionid'];

  insert_new_racer(array('firstname' => $firstname,
                         'lastname' => $lastname,
                         'carnumber' => $_POST['carno'],
                         'carname' => $_POST['carname'],
                         'partitionid' => $partitionid,
                         'exclude' => @$_POST['exclude'] ? true : false));

  json_success();

  $racer_id = read_single_value('SELECT racerid FROM RegistrationInfo'
                                .' WHERE firstname = :firstname'
                                .' AND lastname = :lastname'
                                .' AND partitionid = :partitionid',
                                array(':firstname' => $firstname,
                                      ':lastname' => $lastname,
                                      ':partitionid' => $partitionid));

  json_out('new-row', json_table_row_for_id($racer_id));
} else {
  json_not_authorized();
}
?>