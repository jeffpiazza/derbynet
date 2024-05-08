<?php @session_start();

// $_GET['back'] if "Back" button should go to another page, otherwise kiosk-coordinator.php.

require_once('inc/data.inc');
require_once('inc/authorize.inc');
session_write_close();
require_once('inc/banner.inc');
require_once('inc/schema_version.inc');
require_once('inc/kiosks.inc');
require_once('inc/scenes.inc');

require_permission(SET_UP_PERMISSION);
?><!DOCTYPE html>
<html>
<head>
<title>Scenes</title>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/mobile.css"/>
<link rel="stylesheet" type="text/css" href="css/scenes.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/mobile.js"></script>
<script type='text/javascript' src="js/modal.js"></script>
<script type='text/javascript' src="js/scenes.js"></script>
<script type='text/javascript'>
///////////////////////////////////
// g_all_scene_kiosk_names is a sorted list of kiosk names, e.g.,
//   ["Main","Aux1","Aux2"].
//
// g_all_scenes is an array of e.g.
//   { "sceneid": "4",
//     "name": "Awards",
//     "kiosks": [{ "kiosk_name": "Main",
//                  "page": "kiosks\/award-presentations.kiosk" }]
//   }
//
// g_current_scene is the name of the scene currently being shown on this page.
//
// g_all_pages is an array of e.g.
//   { "brief": "award-presentations",
//     "full": "kiosks\/award-presentations.kiosk" },
///////////////////////////////////
var g_all_scenes = <?php echo json_encode(all_scenes(),
                                          JSON_HEX_TAG | JSON_HEX_AMP | JSON_PRETTY_PRINT); ?>;
var g_all_scene_kiosk_names = <?php echo json_encode(all_scene_kiosk_names(),
                                                     JSON_HEX_TAG | JSON_HEX_AMP); ?>;
var g_current_scene = <?php echo json_encode(read_raceinfo('current_scene', ''),
                                             JSON_HEX_TAG | JSON_HEX_AMP); ?>;
var g_all_pages = <?php echo json_encode(all_kiosk_pages(),
                                         JSON_HEX_TAG | JSON_HEX_AMP | JSON_PRETTY_PRINT); ?>;
</script>
</head>
<body>
<?php make_banner('Scenes', isset($_GET['back']) ? $_GET['back'] : 'kiosk-dashboard.php'); ?>

<div id="select-wrap" class="block_buttons">
 <!-- block_buttons for  .block_buttons option  css rule -->
  <select id="scenes-select">
  </select>
</div>

<div id="previews">
</div>

<div class="block_buttons">
  <input type="button" id="add_kiosk_button" value="Add Kiosk Name"
         onclick='on_add_kiosk()'/>
  <input type="button" id="delete_scene_button" class="delete_button" value="Delete Scene"
         onclick='on_delete_scene()'/>
</div>

<div id="new_scene_modal" class="modal_dialog hidden block_buttons">
  <form>
    <h3>Make New Scene</h3>
    <input type="text" id="new_scene_name"/>
    <input type="submit" value="Create"/>
    <input type="button" value="Cancel"
      onclick='close_modal("#new_scene_modal");'/>
  </form>
</div>

<div id="new_kiosk_modal" class="modal_dialog hidden block_buttons">
  <form>
    <h3>Make New Kiosk Name</h3>
    <input type="text" id="new_kiosk_name"/>
    <input type="submit" value="Create"/>
    <input type="button" value="Cancel"
      onclick='close_modal("#new_kiosk_modal");'/>
  </form>
</div>

</body>
</html>
