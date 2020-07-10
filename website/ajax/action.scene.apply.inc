<?php
// $_POST['sceneid'] OR $_POST['name'] to select the scene
start_response();

if (have_permission(CONTROL_RACE_PERMISSION)) {
  if (isset($_POST['sceneid'])) {
    $sceneid = $_POST['sceneid'];
  } else if (isset($_POST['name'])) {
    $sceneid = read_single_value('SELECT sceneid FROM Scenes WHERE name = :name',
                                 array(':name' => $_POST['name']));
  } else {
    echo "<failure>No scene specified</failure>";
  }

  if (isset($sceneid)) {
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
    echo "<success/>\n";
    require('ajax/query.poll.kiosk.all.inc');
  }
} else {
  not_authorized_body();
}

end_response();
?>