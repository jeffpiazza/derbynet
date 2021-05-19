<?php

$devices = array();
$flags = array();
try {
  $stmt = $db->prepare('SELECT kind, key, type, value, description FROM TimerSettings');
  $stmt->execute(array());
  foreach ($stmt as $row) {
    if ($row['kind'] == 'flag') {
      $flags[] = array('name' => $row['key'],
                       'type' => $row['type'],
                       'value' => $row['value'],
                       'description' => $row['description']);
    } else if ($row['kind'] == 'device') {
      $devices[] = array('name' => $row['key'],
                         'description' => $row['description']);
    } else if ($row['kind'] == 'ports') {  // comma-separated list
      json_out('ports', $row['value']);
    }
  }
} catch (PDOException $p) {
}

json_out('devices', $devices);
json_out('flags', $flags);

?>
