<settings>
<?php

if (isset($_GET['key'])) {
  $stmt = $db->prepare('SELECT itemkey, itemvalue FROM RaceInfo WHERE itemkey = :key');
  $stmt->execute(array(':key' => $_GET['key']));
} else {
  $stmt = $db->prepare('SELECT itemkey, itemvalue FROM RaceInfo');
  $stmt->execute(array());
}
foreach ($stmt as $row) {
  echo '  <setting key=\''.htmlspecialchars($row['itemkey'], ENT_QUOTES, 'UTF-8').'\'>';
  echo htmlspecialchars($row['itemvalue'], ENT_QUOTES, 'UTF-8');
  echo '</setting>'."\n";
}
?>
</settings>
