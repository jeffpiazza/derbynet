<?php

// $_POST['divisionid']

require_once('inc/divisions.inc');

$ok = true;
if (!have_permission(SET_UP_PERMISSION)) {
  json_not_authorized();
  $ok = false;
}

$divisionid = $_POST['divisionid'];
if ($ok && read_single_value('SELECT COUNT(*) FROM RegistrationInfo'
                             .' WHERE divisionid = :divisionid',
                             array(':divisionid' => $divisionid)) > 0) {
  json_failure('in-use', 'Division has racers assigned');
  $ok = false;
}

if ($ok) {
  list($classid, $rankid) = read_single_row('SELECT classid, rankid'
                                            .' FROM Divisions JOIN Ranks USING (rankid)'
                                            .' WHERE divisionid = :divisionid',
                                            array(':divisionid' => $divisionid));

  read_single_value('DELETE FROM Divisions WHERE divisionid = :divisionid',
                    array(':divisionid' => $divisionid));

  // if by-division, delete the corresponding class, if there are no racing results
  // if one-group, delete the corresponding rank.
  switch (group_formation_rule()) {
  case 'one-group':
    // The rank is no longer associated to any division, and so should be deleted.
    if (read_single_value('SELECT COUNT(*) FROM RegistrationInfo WHERE rankid = :rankid',
                          array(':rankid' => $rankid)) == 0) {
      read_single_value('DELETE FROM Ranks WHERE rankid = :rankid',
                        array(':rankid' => $rankid));
    }
    break;
  case 'by-division':
    if (read_single_value('SELECT COUNT(*) FROM RegistrationInfo WHERE classid = :classid',
                          array(':classid' => $classid)) == 0) {
      read_single_value('DELETE FROM Ranks WHERE classid = :classid',
                        array(':classid' => $classid));
      read_single_value('DELETE FROM Classes WHERE classid = :classid',
                        array(':classid' => $classid));
    }
    break;
  }

  json_success();
}