<?php @session_start();
require_once('inc/data.inc');
require_once('inc/banner.inc');
require_once('inc/schema_version.inc');
require_once('inc/authorize.inc');
require_once('inc/kiosks.inc');
require_permission(SET_UP_PERMISSION);

$stmt = $db->prepare('SELECT DISTINCT kiosk_name FROM SceneKiosk ORDER BY '
                     .' CASE kiosk_name WHEN \'Main\' THEN \'\' ELSE kiosk_name END');
$stmt->execute();
$all_kiosk_names = array();
foreach ($stmt as $row) {
  $all_kiosk_names[] = $row['kiosk_name'];
}

$stmt = $db->prepare('SELECT DISTINCT kiosk_name FROM SceneKiosk ORDER BY '
                     .' CASE kiosk_name WHEN \'Main\' THEN \'\' ELSE kiosk_name END');
$stmt->execute();
$all_kiosk_names = array();
foreach ($stmt as $row) {
  $all_kiosk_names[] = $row['kiosk_name'];
}

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
?><!DOCTYPE html>
<html>
<head>
<title>Scenes</title>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<link rel="stylesheet" type="text/css" href="css/scenes.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/pseudo-mobile.js"></script>
<script type='text/javascript' src="js/modal.js"></script>
<script type='text/javascript' src="js/scenes.js"></script>
<script type='text/javascript'>
///////////////////////////////////
// g_all_kiosk_names is a sorted list of kiosk names, e.g.,
//   ["Main","Aux1","Aux2"].
//
// g_all_pages is an array of e.g.
//   { "brief": "award-presentations",
//     "full": "kiosks\/award-presentations.kiosk" },
//
// g_all_scenes is an array of e.g.
//   { "sceneid": "4",
//     "name": "Awards",
//     "kiosks": [{ "kiosk_name": "Main",
//                  "page": "kiosks\/award-presentations.kiosk" }]
//   }
//
// g_current_scene is the name of the scene currently being shown on this page.
///////////////////////////////////
var g_all_kiosk_names = <?php echo json_encode($all_kiosk_names, JSON_HEX_AMP); ?>;
var g_all_pages = <?php echo json_encode(all_kiosk_pages(), JSON_HEX_AMP | JSON_PRETTY_PRINT); ?>;
var g_all_scenes = <?php echo json_encode($all_scenes, JSON_HEX_AMP | JSON_PRETTY_PRINT); ?>;
var g_current_scene = <?php echo json_encode(read_raceinfo('current_scene', ''), JSON_HEX_AMP); ?>;
</script>
</head>
<body>
<?php make_banner('Scenes', 'setup.php'); ?>

<div id="select-wrap">
  <select id="scenes-select">
  </select>
</div>

<div id="previews">
</div>

<div class="block_buttons">
  <input type="button" id="add_kiosk_button" value="Add Kiosk"
         onclick='on_add_kiosk()'/>
  <input type="button" id="delete_scene_button" class="delete_button" value="Delete Scene"
         onclick='on_delete_scene()'/>
</div>

<div id="new_scene_modal" class="modal_dialog hidden block_buttons">
  <form>
    <h3>Make New Scene</h3>
    <input type="text" id="new_scene_name"/>
    <input type="submit" data-enhanced="true" value="Create"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick='close_modal("#new_scene_modal");'/>
  </form>
</div>

<div id="new_kiosk_modal" class="modal_dialog hidden block_buttons">
  <form>
    <h3>Make New Kiosk Name</h3>
    <input type="text" id="new_kiosk_name"/>
    <input type="submit" data-enhanced="true" value="Create"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick='close_modal("#new_kiosk_modal");'/>
  </form>
</div>

</body>
</html>
