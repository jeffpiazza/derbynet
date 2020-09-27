<?php

function create_scene_tables() {
  run_sql_script('scene-tables.inc');
}

// Returns an array of e.g.
//   { "sceneid": "4",
//     "name": "Awards",
//     "kiosks": [{ "kiosk_name": "Main",
//                  "page": "kiosks\/award-presentations.kiosk" }]
//   }
function all_scenes() {
  global $db;
  try {
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
  } catch (PDOException $p) {
    if (is_no_such_table_eception($p)) {
      create_scene_tables();
      return array();
    } else {
      echo "SQL error code ".$p->getCode()."\n";
      echo $p;
    }
  }
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

function apply_scene($sceneid) {
  global $db;
  write_raceinfo('current_scene', $sceneid);
  $upd = $db->prepare('UPDATE Kiosks SET page = :page WHERE name = :name');

  $stmt = $db->prepare('SELECT kiosk_name, page FROM SceneKiosk'
                       .' WHERE sceneid = :sceneid'
                       .' AND PAGE <> \'\'');
  $stmt->execute(array(':sceneid' => $sceneid));
  foreach ($stmt as $row) {
    $upd->execute(array(':name' => $row['kiosk_name'],
                        ':page' => $row['page']));
  }
}

function maybe_apply_racing_scene() {
  $racing_scene = read_raceinfo('racing_scene', '');
  if ($racing_scene) {
    apply_scene($racing_scene);
  }
}

function maybe_apply_playlist_scene() {
  global $db;
  $when = read_raceinfo('playlist_scene_at');
  if ($when !== false && $when < time()) {
    $db->exec('DELETE FROM RaceInfo WHERE itemkey = \'playlist_scene_at\'');
    apply_scene(read_raceinfo('playlist_scene'));
  }
}

?>