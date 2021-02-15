<timer-settings>
<?php

try {
  $stmt = $db->prepare('SELECT kind, key, type, value, description FROM TimerSettings');
  $stmt->execute(array());
  foreach ($stmt as $row) {
    echo "  <$row[kind] name=\"".htmlspecialchars($row['key'], ENT_QUOTES, 'UTF-8')."\"";
    echo " type=\"$row[type]\" value=\"".htmlspecialchars($row['value'], ENT_QUOTES, 'UTF-8')."\"";
    echo ">";
    echo htmlspecialchars($row['description'], ENT_QUOTES, 'UTF-8');
    echo "</$row[kind]>\n";
  }
} catch (PDOException $p) {
}
?>
</timer-settings>