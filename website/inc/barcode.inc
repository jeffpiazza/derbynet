<?php

require_once('inc/car-numbering.inc');
require_once('inc/newracer.inc');

// Returns list(racerid, bool), the bool indicating whether a new racer was created.
// Returns 0 if racer can't be uniquely identified from the barcode identifier.
function barcode_to_racerid($barcode) {
  global $db;
  $barcode = trim($barcode);
  if (substr($barcode, 0, strlen('PWDid')) == 'PWDid') {
    $racerid = intval(substr($barcode, strlen('PWDid')));
    // The database access here is to confirm that the racerid actually
    // identifies a racer.
    return array(read_single_value('SELECT racerid FROM RegistrationInfo'
                                   .' where racerid = :racerid',
                                   array(':racerid' => $racerid), 0),
                 false);
  } else if (substr($barcode, 0, strlen('PWDwi')) == 'PWDwi') {  // walk-in
    $carno = intval(substr($barcode, strlen('PWDwi')));
    $created = false;
    if (read_single_value('SELECT COUNT(*) FROM RegistrationInfo WHERE carnumber = :carno',
                          array(':carno' => $carno)) == 0) {
      list($mult, $smallest) = read_car_numbering_values();
      if ($mult > 0) {
        $century = intdiv($carno - $smallest, $mult) + 1;
        $partitionid = read_single_value('SELECT partitionid FROM Partitions WHERE sortorder = :century',
                                         array(':century' => $century));
      } else {
        $partition = read_single_value('SELECT COALESCE(MAX(partitionid), 1) FROM Partitions');
      }

      insert_new_racer(array('firstname' => 'Walk-In',
                             'lastname' => $carno,
                             'carnumber' => $carno,
                             'partitionid' => $partitionid));
      $created = true;
    }
    // If there are multiple racers with the same car number, you're just out of luck
    return array(read_single_value('SELECT racerid FROM RegistrationInfo WHERE carnumber = :carno',
                                   array(':carno' => $carno)),
                 $created);
  } else if (substr($barcode, 0, strlen('PWD')) == 'PWD') {
    $carno = substr($barcode, strlen('PWD'));
    if (read_single_value('SELECT COUNT(*) FROM RegistrationInfo WHERE carnumber = :carno',
                          array(':carno' => $carno)) == 1) {
      return array(read_single_value('SELECT racerid FROM RegistrationInfo WHERE carnumber = :carno',
                                     array(':carno' => $carno)),
                   false);
    }
  }
  return array(0, false);
}
?>
