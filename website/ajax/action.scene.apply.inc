<?php
// $_POST['sceneid'] OR $_POST['name'] to select the scene

require_once('inc/scenes.inc');

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
    apply_scene($sceneid);
    echo "<success/>\n";
    require('ajax/query.poll.kiosk.all.inc');
  }
} else {
  not_authorized_body();
}

end_response();
?>