<?php

// Returns 0 if racer can't be uniquely identified from the barcode identifier.
//
// Presently the barcode identifier is expected to be 'PWD' followed by one of
// the following forms:
// (a) the literal string "id" followed by the racerid of the racer, or
// (b) the car number with no prefix, which works for the common but not
// universal case where car numbers are uniquely assigned.
function barcode_to_racerid($barcode) {
  global $db;
  $barcode = trim($barcode);
  if (substr($barcode, 0, strlen('PWDid')) == 'PWDid') {
    $racerid = intval(substr($barcode, strlen('PWDid')));
    // The database access here is to confirm that the racerid actually
    // identifies a racer.
    return read_single_value('SELECT racerid FROM RegistrationInfo'
                             .' where racerid = :racerid',
                             array(':racerid' => $racerid), 0);
  } else if (substr($barcode, 0, strlen('PWD')) == 'PWD') {
    $carno = substr($barcode, strlen('PWD'));
    if (read_single_value('SELECT COUNT(*) FROM RegistrationInfo WHERE carnumber = :carno',
                          array(':carno' => $carno)) == 1) {
      return read_single_value('SELECT racerid FROM RegistrationInfo WHERE carnumber = :carno',
                               array(':carno' => $carno));
    }
  }
  return 0;
}
?>