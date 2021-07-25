<?php

require_once('inc/schema_version.inc');

// divisions, divseq, subdivisions, subdivseq
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
    if ($row['divisionid'] != $prev_div['divisionid']) {
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


  
  // SELECT DISTINCT rankid, divisionid FROM RegistrationInfo
  
  $stmt = $db->prepare('SELECT divisionid, name'
                       .' FROM Divisions'
                       .' ORDER BY Divisions.sortorder');
  $stmt->execute();
  $divisions = array();
  $divseq = array();
  foreach ($stmt as $div) {
    $div['ranks'] = array();
    $divisions[$div['divisionid']] = $div;
    $divseq[] = $div['divisionid'];
  }

  $stmt = $db->prepare('SELECT subdivisionid, Subdivisions.name, superdiv'
                       .' FROM Subdivisions JOIN Divisions'
                       .'  ON superdiv = divisionid'
                       .' ORDER BY Divisions.sortorder, Subdivisions.sortorder');
  $stmt->execute();
  $subdivisions = array();
  $subdivseq = array();
  foreach ($stmt as $sub) {
    $sub['ranks'] = array();
    $subdivisions[$sub['subdivisionid']] = $sub;
    $subdivseq[] = $sub['subdivisionid'];
  }

  $stmt = $db->prepare('SELECT DISTINCT rankid, divisionid FROM RegistrationInfo');
  $stmt->execute();
  foreach($stmt as $rank_div) {
    $divisions[$rank_div['divisionid']]['ranks'][] = $rank_div['rankid'];
  }

  $stmt = $db->prepare('SELECT DISTINCT rankid, subdivisionid FROM RegistrationInfo');
  $stmt->execute();
  foreach($stmt as $rank_div) {
    $subdivisions[$rank_div['subdivisionid']]['ranks'][] = $rank_div['rankid'];
  }

  return array($divisions, $divseq, $subdivisions, $subdivseq);
}

?>