<?php

require_once('inc/schema_version.inc');


function json_divisions() {
  global $db;

  $stmt = $db->prepare('SELECT divisionid, name, sortorder FROM Divisions ORDER BY sortorder');
  $stmt->execute();

  $divisions = array();
  $by_divid = array();
  
  $last_div = array('divisionid' => -1);
  foreach ($stmt as $row) {
    if ($row['divisionid'] != $last_div['divisionid']) {
      $divisions[] = array('divisionid' => $row['divisionid'],
                           'name' => $row['name'],
                           'sortorder' => $row['sortorder'],
                           'classids' => array(),
                           'rankids' => array());
      $last_div = &$divisions[count($divisions) - 1];
      $by_divid[$row['divisionid']] = &$last_div;
    }
  }

  $stmt = $db->prepare('SELECT DISTINCT rankid, classid, divisionid FROM RegistrationInfo');
  $stmt->execute();
  foreach ($stmt as $row) {
    $by_divid[$row['divisionid']]['classids'][] = $row['classid'];
    $by_divid[$row['divisionid']]['rankids'][] = $row['rankid'];
  }

  return $divisions;
}

?>