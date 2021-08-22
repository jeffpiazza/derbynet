<?php

// $_POST[divisionid_(ord)] = divisionid, (ord) = 1,2,3,...

// Reordering divisions should reorder the corresponding groups (by-division) or subgroups (one-group).

require_once('inc/divisions.inc');

$ok = true;
if (!have_permission(SET_UP_PERMISSION)) {
  json_not_authorized();
  $ok = false;
}

if ($ok) {
  $stmt = $db->prepare('UPDATE Divisions SET sortorder = :sortorder WHERE divisionid = :divisionid');
  foreach ($_POST as $key => $val) {
    if (substr($key, 0, strlen('divisionid_')) == 'divisionid_') {
      $stmt->execute(array(':sortorder' => substr($key, strlen('divisionid_')),
                           ':divisionid' => $val));
    }
  }

  switch (group_formation_rule()) {
  case 'by-division':
    read_single_value('UPDATE Classes SET sortorder = (SELECT Divisions.sortorder'
                      .'                             FROM '.inner_join('Divisions', 'Ranks',
                                                                       'Divisions.rankid = Ranks.rankid')
                      .'                             WHERE Ranks.classid = Classes.classid)');
    break;
  case 'one-group':
    read_single_value('UPDATE Ranks SET sortorder = (SELECT Divisions.sortorder FROM Divisions'
                      .'                             WHERE Divisions.rankid = Ranks.rankid)');
    break;
  }
  
  json_success();
}

?>