<?php

start_response();

$stmt = $db->prepare('SELECT sceneid, name, kiosk_name, page'
                     .' FROM Scenes INNER JOIN SceneKiosk USING (sceneid)'
                     .' ORDER BY Scenes.sortorder, kiosk_name');
$stmt->execute();
$last_sceneid = -1;
foreach ($stmt as $row) {
  if ($row['sceneid'] != $last_sceneid) {
    if ($last_sceneid >= 0) {
      echo "</scene>\n";
    }
    echo  "<scene name=\"".htmlspecialchars($row['name'], ENT_QUOTES, 'UTF-8')."\">\n";
    $last_sceneid = $row['sceneid'];
  }

  echo "  <kiosk"
          ." kiosk_name=\"".htmlspecialchars($row['kiosk_name'], ENT_QUOTES, 'UTF-8')."\""
          ." page=\"".htmlspecialchars($row['page'], ENT_QUOTES, 'UTF-8')."\""
          ."/>\n";
}
if ($last_sceneid >= 0) {
  echo "</scene>\n";
}

echo "<success/>\n";

end_response();
?>