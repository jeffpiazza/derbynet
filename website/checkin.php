<?php @session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/banner.inc');
require_once('inc/authorize.inc');
require_once('inc/schema_version.inc');
require_once('inc/photo-config.inc');
require_once('inc/classes.inc');
require_once('inc/checkin-table.inc');

require_permission(CHECK_IN_RACERS_PERMISSION);

// This is the racer check-in page.  It appears as a table of all the registered
// racers, with a flipswitch for each racer.  Clicking on the check-in button
// invokes some javascript that sends an ajax POST request to check-in (or
// un-check-in) that racer.  See checkin.js.

// In addition to the actual check-in, it's possible to change a
// racer's car number from this form, or mark the racer for our
// "exclusively by scout" award.

// Here on the server side, a GET request sends HTML for the whole
// page.  POST requests to make changes to the database are sent to
// action.php, and produce just a small XML document.

// TODO- subgroups explanation

$use_groups = use_groups();
$use_subgroups = use_subgroups();

// Our pack provides an "exclusively by scout" award, based on a
// signed statement from the parent.  Collecting the statement is part
// of the check-in process, so there's provision for a checkbox on the
// check-in form.  For groups that don't do this, $xbs will be false
// (and $xbs_award_name will be blank), and the checkboxes won't be
// shown.
$xbs = read_raceinfo_boolean('use-xbs');
$xbs_award_name = read_raceinfo('xbs-award', 'Exclusively By Scout');

$order = '';
if (isset($_GET['order']))
  $order = $_GET['order'];  // Values are: name, class, car
if (!$order)
    $order = 'name';

// The table of racers can be presented in order by name, car, or class (and
// then by name within the class).  Each sortable column has a header which is a
// link to change the ordering, with the exception that the header for the
// column for ordering currently in use is NOT a link (because it wouldn't do
// anything).
function column_header($text, $o) {
  global $order;
  return "<a data-order='".$o."' "
      .($o == $order ? "" : " href='#'").">".$text."</a>";
}
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Check-In</title>
<link rel="stylesheet" type="text/css" href="css/dropzone.min.css"/>
<link rel="stylesheet" type="text/css" href="css/mobile.css"/>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/main-table.css"/>
<link rel="stylesheet" type="text/css" href="css/checkin.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/ajax-setup.js"></script>
<script type="text/javascript">
var g_order = '<?php echo $order; ?>';
var g_action_on_barcode = "<?php
  echo isset($_SESSION['barcode-action']) ? $_SESSION['barcode-action'] : "locate";
?>";
</script>
<script type="text/javascript" src="js/mobile.js"></script>
<script type="text/javascript" src="js/dashboard-ajax.js"></script>
<script type="text/javascript" src="js/modal.js"></script>
<script type="text/javascript" src="js/webcam.js"></script>
<script type="text/javascript" src="js/dropzone.min.js"></script>
<script type="text/javascript" src="js/checkin.js"></script>
</head>
<body>
<?php
make_banner('Racer Check-In');
?>

<div class="block_buttons">
  <img src="img/barcode.png" style="position: absolute; left: 16px; top: 80px;"
      onclick="handle_barcode_button_click()"/>

  <input class="bulk_button"
        type="button" value="Bulk"
        onclick='show_bulk_form();'/>
<?php if (have_permission(REGISTER_NEW_RACER_PERMISSION)) { ?>
      <input type="button" value="New Racer"
        onclick='show_new_racer_form();'/>
<?php } ?>
</div>

<table id="main_checkin_table" class="main_table">
<thead>
  <tr>
    <th/>
    <?php if ($use_groups) {
        echo '<th>'.column_header(group_label(), 'class').'</th>';
    } ?>
    <?php if ($use_subgroups) {
        echo '<th>'.subgroup_label().'</th>';
    } ?>
    <th><?php echo column_header('Car Number', 'car'); ?></th>
    <th>Photo</th>
    <th><?php echo column_header('Last Name', 'name'); ?></th>
    <th>First Name</th>
    <th>Car Name</th>
    <th>Passed?</th>
    <?php if ($xbs) {
        echo '<th>'.$xbs_award_name.'</th>';
    } ?>
  </tr>
</thead>

<tbody>
<?php

  list($classes, $classseq, $ranks, $rankseq) = classes_and_ranks();

  $sql = 'SELECT racerid, carnumber, lastname, firstname, carname, imagefile,'
      .(schema_version() < 2 ? "" : " carphoto,")
      .(schema_version() < 2 ? "class" : "Classes.sortorder").' AS class_sort,'
      .(schema_version() < 2 ? "rank" : "Ranks.sortorder").' AS rank_sort,'
      .' RegistrationInfo.classid, class, RegistrationInfo.rankid, rank, passedinspection, exclude,'
      .' EXISTS(SELECT 1 FROM RaceChart WHERE RaceChart.racerid = RegistrationInfo.racerid) AS scheduled,'
      .' EXISTS(SELECT 1 FROM RaceChart WHERE RaceChart.classid = RegistrationInfo.classid) AS denscheduled,'
      .' EXISTS(SELECT 1 FROM Awards WHERE Awards.awardname = \''.addslashes($xbs_award_name).'\' AND'
      .'                                   Awards.racerid = RegistrationInfo.racerid) AS xbs'
      .' FROM '.inner_join('RegistrationInfo', 'Classes',
                           'RegistrationInfo.classid = Classes.classid',
                           'Ranks',
                           'RegistrationInfo.rankid = Ranks.rankid')
      .' ORDER BY '
          .($order == 'car' ? 'carnumber, lastname, firstname' :
            ($order == 'class'  ? 'class_sort, rank_sort, lastname, firstname' :
             'lastname, firstname'));

$stmt = $db->query($sql);

$n = 1;
foreach ($stmt as $rs) {
  // TODO
  $rs['rankseq'] = $ranks[$rs['rankid']]['seq'];
  checkin_table_row($rs, $use_groups, $use_subgroups, $xbs, $n);
  ++$n;
}
?>
</tbody>
</table>
<div class="block_buttons">
<?php if (have_permission(REGISTER_NEW_RACER_PERMISSION)) { ?>
      <input type="button" value="New Racer"
        onclick='show_new_racer_form();'/>
<?php } ?>
</div>


<div id='edit_racer_modal' class="modal_dialog hidden block_buttons">
<form id="editracerform">

  <input id="edit_racer" type="hidden" name="racer" value=""/>

  <label for="edit_firstname">First name:</label>
  <input id="edit_firstname" type="text" name="edit_firstname" value=""/>
  <label for="edit_lastname">Last name:</label>
  <input id="edit_lastname" type="text" name="edit_lastname" value=""/>

  <label for="edit_carno">Car number:</label>
  <input id="edit_carno" type="text" name="edit_carno" value=""/>
  <br/>

  <label for="edit_carname">Car name:</label>
  <input id="edit_carname" type="text" name="edit_carname" value=""/>
  <br/>

<?php
// $rank_options is a string of <option> elements, one per rank.
//    $rank_options is used to pick a rank for a new or modified racer.
//
// $ranks_and_classes is a string of <option> elements, one for each rank and each class.
//    $ranks_and_classes is used for picking groups of racers to which a bulk operation
//    might apply.

    $rank_options = "";
    $ranks_and_classes = "";
    $last_classid = 0;
    $sql = 'SELECT rankid, rank, Ranks.classid, class'
           .' FROM Ranks INNER JOIN Classes'
           .' ON Ranks.classid = Classes.classid'
           .' ORDER BY '
           .(schema_version() >= 2 ? 'Classes.sortorder, Ranks.sortorder, ' : '')
           .'class, rank';
    $stmt = $db->query($sql);
    foreach ($stmt as $rs) {
      $rank_options .= "\n".'<option value="'.$rs['rankid'].'"'
            .' data-class="'.htmlspecialchars($rs['class'], ENT_QUOTES, 'UTF-8').'"'
           .' data-rank="'.htmlspecialchars($rs['rank'], ENT_QUOTES, 'UTF-8').'"'
           .'>'
           .htmlspecialchars($rs['class'], ENT_QUOTES, 'UTF-8')
          .' / '.htmlspecialchars($rs['rank'], ENT_QUOTES, 'UTF-8')
	       .'</option>';
      if ($rs['classid'] != $last_classid) {
        $ranks_and_classes .= "\n".'<option value="c'.$rs['classid'].'">'
                .htmlspecialchars($rs['class'], ENT_QUOTES, 'UTF-8')
                .'</option>';
      }
      if ($use_subgroups) {
        $ranks_and_classes .= "\n".'<option value="r'.$rs['rankid'].'">'
                .htmlspecialchars($rs['rank'], ENT_QUOTES, 'UTF-8')
                .'</option>';
      }
      $last_classid = $rs['classid'];
    }

    if (!$rank_options) {
      echo "<div id='rank_missing'>\n";
      echo "<img src='img/status/trouble.png'/>\n";
      echo "<p>There are no racing groups defined.  "
          ."Visit the <a href='class-editor.php'>'Edit ".group_label()."'</a>"
          ." page to define racing groups.</p>\n";
      echo "</div>";
    }
?>
  <label for="edit_rank">Racing group:</label>
    <select id="edit_rank"><?php echo $rank_options; ?>
    </select>
  <br/>
  <label for="eligible">Trophy eligibility:</label>
    <input type="checkbox" class="flipswitch" name="eligible" id="eligible"
            data-wrapper-class="trophy-eligible-flipswitch"
            data-off-text="Excluded"
            data-on-text="Eligible"/>
  <br/>
  <input type="submit"/>
  <input type="button" value="Cancel"
    onclick='close_modal("#edit_racer_modal");'/>

  <div id="delete_racer_extension">
    <input type="button" value="Delete Racer"
           class="delete_button"
           onclick="handle_delete_racer();"/>
  </div>
</form>
</div>

<div id='photo_modal' class="modal_dialog hidden block_buttons">
  <form id="photo_drop" class="dropzone">
    <input type="hidden" name="action" value="photo.upload"/>
    <input type="hidden" id="photo_modal_repo" name="repo"/>
    <input type="hidden" id="photo_modal_racerid" name="racerid"/>
    <input type="hidden" name="MAX_FILE_SIZE" value="30000000" />

    <h3>Capture <span id="racer_photo_repo"></span> photo for <span id="racer_photo_name"></span></h3>
    <div id="preview">
        <h2>Does your browser support webcams?</h2>
    </div>

    <?php
      if (headshots()->status() != 'ok') {
        echo '<p class="warning">Check <a href="settings.php">photo directory settings</a> before proceeding!</p>';
      }
    ?>

    <div class="block_buttons">
        <input type="submit" value="Capture &amp; Check In" id="capture_and_check_in"
           onclick='g_check_in = true;'/>
        <br/>
        <input type="submit" value="Capture Only"
          onclick='g_check_in = false;'/>
        <input type="button" value="Switch Camera"
          onclick='handle_switch_camera();'/>
        <input type="button" value="Cancel"
          onclick='close_photo_modal();'/>

        <label id="autocrop-label" for="autocrop">Auto-crop after upload:</label>
        <div class="centered_flipswitch">
          <input type="checkbox" class="flipswitch" name="autocrop" id="autocrop" checked="checked"/>
        </div>
    </div>
    <div class="dz-message"><span>NOTE: You can drop a photo here to upload instead</span></div>
  </form>
</div>


<div id='bulk_modal' class="modal_dialog hidden block_buttons">
  <input type="button" value="Bulk Check-In"
    onclick="bulk_check_in(true);"/>
  <input type="button" value="Bulk Check-In Undo"
    onclick="bulk_check_in(false);"/>
  <br/>
  <input type="button" value="Bulk Numbering"
    onclick="bulk_numbering();"/>
  <input type="button" value="Bulk Eligibility"
    onclick="bulk_eligibility();"/>
  <br/>
  <input type="button" value="Cancel"
    onclick='close_modal("#bulk_modal");'/>
</div>

<div id="bulk_details_modal" class="modal_dialog hidden block_buttons">
    <form id="bulk_details">
      <h2 id="bulk_details_title"></h2>

      <label id="who_label" for="bulk_who">Assign car numbers to</label>
      <select id="bulk_who">
        <option value="all">All</option>
        <?php echo $ranks_and_classes; ?>
      </select>

      <div id="numbering_controls" class="hidable">
        <label for="bulk_numbering_start">Starting from:</label>
        <input type="number" id="bulk_numbering_start" name="bulk_numbering_start"
               value="101"/>

        <label for="renumber">Renumber cars that already have numbers?</label>
          <input type="checkbox" class="flipswitch renumber-flip" name="renumber" id="renumber"
                 data-on-text="Yes" data-off-text="No"    />
      </div>

      <div id="elibility_controls" class="hidable">
        <label for="bulk_eligible">Trophy eligibility:</label>
        <input type="checkbox" class="flipswitch eligible-flip"
               checked="checked"
               name="bulk_eligible" id="bulk_eligible"
               data-wrapper-class="trophy-eligible-flipswitch"
               data-off-text="Excluded"
               data-on-text="Eligible"/>
      </div>
    
      <input type="submit"/>
      <input type="button" value="Cancel"
        onclick='close_secondary_modal("#bulk_details_modal");'/>
    </form>
</div>


<div id="barcode_settings_modal" class="modal_dialog hidden block_buttons">
  <form>
    <h2>Barcode Responses</h2>
    <input id="barcode-handling-locate" name="barcode-handling" type="radio" value="locate"/>
    <label for="barcode-handling-locate">Locate racer</label>
    <input id="barcode-handling-checkin" name="barcode-handling" type="radio" value="checkin"/>
    <label for="barcode-handling-checkin">Check in racer</label>
    <input id="barcode-handling-racer" name="barcode-handling" type="radio" value="racer-photo"/>
    <label for="barcode-handling-racer">Capture racer photo</label>
    <input id="barcode-handling-car" name="barcode-handling" type="radio" value="car-photo"/>
    <label for="barcode-handling-car">Capture car photo</label>

    <input type="submit" value="Close"/>
  </form>
</div>

<?php require_once('inc/ajax-pending.inc'); ?>
<div id="find-racer">
  <div id="find-racer-form">
    Find Racer:
    <input type="text" id="find-racer-text" name="narrowing-text" class="not-mobile"/>
    <span id="find-racer-message"><span id="find-racer-index" data-index="1">1</span> of <span id="find-racer-count">0</span></span>
    <img onclick="cancel_find_racer()" src="img/cancel-20.png"/>
  </div>
</div>
</body>
</html>
