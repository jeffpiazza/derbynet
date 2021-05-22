<?php

if (isset($_GET['key'])) {
  $stmt = $db->prepare('SELECT itemkey, itemvalue FROM RaceInfo WHERE itemkey = :key');
  $stmt->execute(array(':key' => $_GET['key']));
} else {
  $stmt = $db->prepare('SELECT itemkey, itemvalue FROM RaceInfo');
  $stmt->execute(array());
}
$settings = array();
foreach ($stmt as $row) {
  $settings[] = array('key' => $row['itemkey'],
                      'value' => $row['itemvalue']);
}
json_out('settings', $settings);
?>
