<?php

// Returns an array of e.g.
//   { "sceneid": "4",
//     "name": "Awards",
//     "kiosks": [{ "kiosk_name": "Main",
//                  "page": "kiosks\/award-presentations.kiosk" }]
//   }
function all_scenes() {
  global $db;
  $stmt = $db->prepare('SELECT sceneid, name, kiosk_name, page'
                       .' FROM Scenes LEFT JOIN SceneKiosk USING (sceneid)'
                       .' ORDER BY Scenes.sortorder,'
                       .' CASE kiosk_name WHEN \'Main\' THEN \'\' ELSE kiosk_name END');
  $stmt->execute();
  $all_scenes = [];
  $scene = array();
  foreach ($stmt as $row) {
    if (!isset($scene['sceneid']) || $row['sceneid'] != $scene['sceneid']) {
      if (isset($scene['sceneid'])) {
        $all_scenes[] = $scene;
      }
      $scene = array('sceneid' => $row['sceneid'],
                     'name' => $row['name'],
                     'kiosks' => array());
    }

    if ($row['kiosk_name']) {
      $scene['kiosks'][] = array('kiosk_name' => $row['kiosk_name'],
                                 'page' => $row['page']);
    }
  }
  if (isset($scene['sceneid'])) {
    $all_scenes[] = $scene;
  }

  return $all_scenes;
}

// Returns a sorted array of kiosk names used for scenes
function all_scene_kiosk_names() {
  global $db;
  $stmt = $db->prepare('SELECT DISTINCT kiosk_name FROM SceneKiosk ORDER BY '
                       .' CASE kiosk_name WHEN \'Main\' THEN \'\' ELSE kiosk_name END');
  $stmt->execute();
  $all_kiosk_names = array();
  foreach ($stmt as $row) {
    $all_kiosk_names[] = $row['kiosk_name'];
  }
  return $all_kiosk_names;
}
?>