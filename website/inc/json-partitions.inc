<?php

require_once('inc/schema_version.inc');


function json_partitions() {
  global $db;

  $stmt = $db->prepare('SELECT partitionid, name, sortorder,'
                       .' (SELECT COUNT(*) FROM RegistrationInfo'
                       .'  WHERE partitionid = Partitions.partitionid) AS count'
                       .' FROM Partitions ORDER BY sortorder');
  $stmt->execute();

  $partitions = array();
  $by_divid = array();
  
  $last_div = array('partitionid' => -1);
  foreach ($stmt as $row) {
    if ($row['partitionid'] != $last_div['partitionid']) {
      $partitions[] = array('partitionid' => $row['partitionid'],
                            'name' => $row['name'],
                            'sortorder' => $row['sortorder'],
                            'count' => $row['count'],
                            'classids' => array(),
                            'rankids' => array());
      $last_div = &$partitions[count($partitions) - 1];
      $by_divid[$row['partitionid']] = &$last_div;
    }
  }

  $stmt = $db->prepare('SELECT DISTINCT rankid, classid, partitionid FROM RegistrationInfo');
  $stmt->execute();
  foreach ($stmt as $row) {
    $by_divid[$row['partitionid']]['classids'][] = $row['classid'];
    $by_divid[$row['partitionid']]['rankids'][] = $row['rankid'];
  }

  return $partitions;
}

?>