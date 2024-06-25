<?php

require_once('inc/car-numbering.inc');
require_once('inc/checkin-table.inc');
require_once('inc/newracer.inc');

// TODO partitionid < 0 or otherwise out of bounds.  "Default" is the partition
// name used in racer.import if no partition name provided.

if (have_permission(REGISTER_NEW_RACER_PERMISSION)) {
  $firstname = $_POST['firstname'];
  $lastname = $_POST['lastname'];

  $options = array('firstname' => $firstname,
                   'lastname' => $lastname,
                   'carnumber' => $_POST['carno'],
                   'carname' => $_POST['carname'],
                   'note_from' => $_POST['note_from'],
                   'partitionid' => $partitionid,
                   'exclude' => @$_POST['exclude'] ? true : false);
  if (isset($_POST['partitionid']) && $_POST['partitionid'] > 0) {
    $options['partitionid'] = $_POST['partitionid'];
  }
  
  $racer_id = insert_new_racer($options);


  json_success();

  json_out('new-row', json_table_row_for_id($racer_id));
  json_out('car-numbers', next_car_numbers());
} else {
  json_not_authorized();
}
?>
