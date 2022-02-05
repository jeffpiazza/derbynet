<?php

$settings = array();

if (isset($_GET['key'])) {
  $stmt = $db->prepare('SELECT itemvalue FROM RaceInfo WHERE itemkey = :key');
  foreach (explode(',', $_GET['key']) as $key) {
    $stmt->execute(array(':key' => $key));
    $settings[$key] = $stmt->fetch(PDO::FETCH_NUM)[0];
  }
} else {
  $stmt = $db->prepare('SELECT itemkey, itemvalue FROM RaceInfo');
  $stmt->execute(array());
  foreach ($stmt as $row) {
    $settings[] = array('key' => $row['itemkey'],
                        'value' => $row['itemvalue']);
  }
}

json_out('settings', $settings);

?>
