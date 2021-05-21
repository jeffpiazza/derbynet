<?php

$stmt = $db->prepare('SELECT sceneid, name, kiosk_name, page'
                     .' FROM Scenes INNER JOIN SceneKiosk USING (sceneid)'
                     .' ORDER BY Scenes.sortorder, kiosk_name');
$stmt->execute();

$scenes = array();
$last_scene = array();
$last_sceneid = -1;
foreach ($stmt as $row) {
  if ($row['sceneid'] != $last_sceneid) {
    $scenes[] = array('name' => $row['name'],
                      'kiosks' => array());
    $last_scene = &$scenes[count($scenes) - 1];
    $last_sceneid = $row['sceneid'];
  }

  $last_scene['kiosks'][] = array('kiosk_name' => $row['kiosk_name'],
                                  'page' => $row['page']);
}

json_out('scenes', $scenes);

json_success();
?>