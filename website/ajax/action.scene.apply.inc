<?php
// $_POST['sceneid'] OR $_POST['name'] to select the scene

require_once('inc/kiosks.inc');
require_once('inc/scenes.inc');

if (have_permission(CONTROL_RACE_PERMISSION)) {
  if (isset($_POST['sceneid'])) {
    $sceneid = $_POST['sceneid'];
  } else if (isset($_POST['name'])) {
    $sceneid = read_single_value('SELECT sceneid FROM Scenes WHERE name = :name',
                                 array(':name' => $_POST['name']));
  } else {
    json_failure('no-scene', "No scene specified");
  }

  if (isset($sceneid)) {
    if ($sceneid < 0) $sceneid = '';
    apply_scene($sceneid);

    json_success();

    json_out('current-scene', read_raceinfo('current-scene', ''));

    json_out('kiosks', all_kiosks());
    json_out('kiosk-pages', all_kiosk_pages());
  }
} else {
  json_not_authorized();
}
?>