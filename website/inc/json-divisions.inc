<?php

require_once('inc/schema_version.inc');


function json_divisions() {
  global $db;

  $stmt = $db->prepare('SELECT divisionid, Divisions.name as div_name,'
                       .'      subdivisionid, Subdivisions.name as sub_name'
                       .' FROM Divisions LEFT OUTER JOIN Subdivisions'
                       .'  ON divisionid = superdiv'
                       .' ORDER BY Divisions.sortorder, Subdivisions.sortorder');
  $stmt->execute();

  $divisions = array();
  $by_divid = array();
  $by_subdivid = array();
  
  $last_div = array('divisionid' => -1);
  foreach ($stmt as $row) {
    if ($row['divisionid'] != $last_div['divisionid']) {
      $divisions[] = array('divisionid' => $row['divisionid'],
                           'name' => $row['div_name'],
                           'subdivisions' => array(),
                           'classids' => array(),
                           'rankids' => array());
      $last_div = &$divisions[count($divisions) - 1];
      $by_divid[$row['divisionid']] = &$last_div;
    }
    if (isset($row['subdivisionid'])) {
      $last_div['subdivisions'][] = array('subdivisionid' => $row['subdivisionid'],
                                          'name' => $row['sub_name'],
                                          'classids' => array(),
                                          'rankids' => array());
      $by_subdivid[$row['subdivisionid']] = &$last_div['subdivisions'][count($last_div['subdivisions']) - 1];
    }
  }

  $stmt = $db->prepare('SELECT DISTINCT rankid, classid, divisionid FROM RegistrationInfo');
  $stmt->execute();
  foreach ($stmt as $row) {
    $by_divid[$row['divisionid']]['classids'][] = $row['classid'];
    $by_divid[$row['divisionid']]['rankids'][] = $row['rankid'];
  }

  $stmt = $db->prepare('SELECT DISTINCT rankid, classid, subdivisionid FROM RegistrationInfo');
  $stmt->execute();
  foreach ($stmt as $row) {
    $by_subdivid[$row['subdivisionid']]['classids'][] = $row['classid'];
    $by_subdivid[$row['subdivisionid']]['rankids'][] = $row['rankid'];
  }

  return $divisions;
}

?>