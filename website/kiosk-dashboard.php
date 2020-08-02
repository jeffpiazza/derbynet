<?php session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/banner.inc');
require_once('inc/authorize.inc');
require_once('inc/scenes.inc');
require_once('inc/schema_version.inc');
require_once('inc/standings.inc');

require_permission(PRESENT_AWARDS_PERMISSION);

?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Kiosks</title>
<link rel="stylesheet" type="text/css" href="css/jquery-ui.min.css"/>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/ajax-setup.js"></script>
<script type="text/javascript" src="js/jquery-ui.min.js"></script>
<link rel="stylesheet" type="text/css" href="css/kiosk-dashboard.css"/>
<script type="text/javascript" src="js/mobile-init.js"></script>
<script type="text/javascript" src="js/jquery.mobile-1.4.2.min.js"></script>
<script type="text/javascript" src="js/dashboard-ajax.js"></script>
<script type="text/javascript" src="js/modal.js"></script>
<script type="text/javascript" src="js/kiosk-dashboard.js"></script>
<script type="text/javascript">
var g_all_scenes = <?php echo json_encode(all_scenes(),
                                          JSON_HEX_TAG | JSON_HEX_AMP | JSON_PRETTY_PRINT); ?>;
var g_current_scene = <?php echo json_encode(read_raceinfo('current_scene', ''),
                                             JSON_HEX_TAG | JSON_HEX_AMP); ?>;
var g_all_scene_kiosk_names = <?php echo json_encode(all_scene_kiosk_names(),
                                                     JSON_HEX_TAG | JSON_HEX_AMP); ?>;
</script>
</head>
<body>
<?php make_banner('Kiosks'); ?>
<?php require_once('inc/ajax-failure.inc'); ?>

<div id="scenes-control" class="hidden">
  <label for="scenes-select">Current scene:</label>
  <div id="select-wrap">
    <select id="scenes-select"></select>
  </div>
  <div id="scenes-status-message"></div>
</div>

<div class="standings-control hidden control_group block_buttons">
  <div class="round-select">
    <h3>Display standings for:</h3>
    <select id='standings-catalog'>
      <?php
        // This <select> elements lets the operator choose what standings should be displayed on
        // kiosks displaying standings.
        $standings_state =  explode('-', read_raceinfo('standings-message'), 2);
        $current_exposed = $current_catalog_entry = '';
        if (count($standings_state) >= 2) {
          $current_exposed = $standings_state[0];
          $current_catalog_entry = $standings_state[1];
        }

        if ($current_exposed === '') {
          $current_exposed = 'all';
        } else {
          $current_exposed = 'lowest '.$current_exposed;
        }

        $use_subgroups = use_subgroups();

        if ($current_catalog_entry == '') {
          echo '<option selected="selected" disabled="1">Please choose what standings to display</option>';
        }
    {
      foreach (standings_catalog() as $entry) {
        $json_entry = json_encode($entry);
        echo '<option data-catalog-entry="'.htmlspecialchars($json_entry, ENT_QUOTES, 'UTF-8').'"';
        if ($current_catalog_entry == $json_entry) {
          echo ' selected="selected"';
        }
        echo '>';
        echo htmlspecialchars($entry['name'], ENT_QUOTES, 'UTF-8');
        echo "</option>\n";
      }
    }
      ?>
    </select>
  </div>
  <div class="reveal block_buttons">
        <h3 <?php if ($standings_state == '') { echo "class='hidden'"; } ?>>Revealing <span id="current_exposed"><?php echo $current_exposed; ?></span> standing(s).</h3>
    <input type="button" data-enhanced="true" value="Reveal One" onclick="handle_reveal1()"/><br/>
    <input type="button" data-enhanced="true" value="Reveal All" onclick="handle_reveal_all()"/><br/>
  </div>
</div><!-- standings-control -->

<div id="kiosk_control_group" class="kiosk_control_group">
</div>


<div id='kiosk_modal' class="modal_dialog hidden block_buttons">
  <form>
    <label for="kiosk_name_field">Name for kiosk:</label>
    <div id="preferred_kiosk_names"></div>
    <input type="text" id="kiosk_name_field"/>
    <input type="submit" data-enhanced="true" value="Assign"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick='close_modal("#kiosk_modal");'/>
  </form>
</div>

<div id='config_classes_modal' class="modal_dialog hidden block_buttons">
  <form>
    <div id="title_div">
      <label for="title_text">Title text:</label>
      <input type="text" id="title_text"/>
    </div>
    <?php
        $stmt = $db->prepare('SELECT classid, class'
                             .' FROM Classes'
                             .' WHERE EXISTS(SELECT 1 FROM RegistrationInfo'
                             .'              WHERE RegistrationInfo.classid = Classes.classid)'
                             .' ORDER BY '.(schema_version() >= 2
                                            ? 'sortorder, ' : '')
                             .'class');
        $stmt->execute(array());

        foreach ($stmt as $row) {
          echo '<input type="checkbox" name="class-'.$row['classid'].'"'
               .' id="config-class-'.$row['classid'].'"'
               .' data-classid="'.$row['classid'].'"'
               .'/>'."\n";
          echo '<label for="config-class-'.$row['classid'].'">'
              .htmlspecialchars($row['class'], ENT_QUOTES, 'UTF-8')
              .'</label>'."\n";
        }
    ?>
    <input type="submit" data-enhanced="true" value="Configure Kiosk"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick='close_modal("#config_classes_modal");'/>
  </form>
</div>

</body>
</html>