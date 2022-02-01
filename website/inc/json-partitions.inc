<?php

require_once('inc/schema_version.inc');


function json_partitions() {
  global $db;

  $stmt = $db->prepare('SELECT partitionid, Partitions.name AS name,'
                       .' rankid, classid, Partitions.sortorder AS sortorder,'
                       .' (SELECT COUNT(*) FROM RegistrationInfo'
                       .'  WHERE partitionid = Partitions.partitionid) AS count'
                       .' FROM Partitions'
                       .' JOIN Ranks USING (rankid)'
                       .' JOIN Classes USING (classid)'
                       .' ORDER BY sortorder');
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
                            'classids' => array($row['classid']),
                            'rankids' => array($row['rankid']));
      $last_div = &$partitions[count($partitions) - 1];
      $by_divid[$row['partitionid']] = &$last_div;
    }
  }

  return $partitions;
}

?>