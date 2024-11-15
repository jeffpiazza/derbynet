<?php @session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/authorize.inc');
session_write_close();
require_once('inc/banner.inc');
require_once('inc/car-numbering.inc');
require_once('inc/schema_version.inc');
require_once('inc/photo-config.inc');
require_once('inc/classes.inc');
require_once('inc/partitions.inc');
require_once('inc/locked.inc');
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

// Our pack provides an "exclusively by scout" award, based on a
// signed statement from the parent.  Collecting the statement is part
// of the check-in process, so there's provision for a checkbox on the
// check-in form.  For groups that don't do this, $xbs will be false
// (and $xbs_award_name will be blank), and the checkboxes won't be
// shown.
$xbs = read_raceinfo_boolean('use-xbs');
$xbs_award_name = xbs_award();

$order = '';
if (isset($_GET['order']) && in_array($_GET['order'], ['name', 'class', 'car', 'partition']))
  $order = $_GET['order'];
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
<script type="text/javascript" src="js/jquery-ui.min.js"></script>
<script type="text/javascript" src="js/qrcode.min.js"></script>
<script type="text/javascript" src="js/ajax-setup.js"></script>
<script type="text/javascript">
var g_order = '<?php echo $order; ?>';
var g_action_on_barcode = "<?php
  echo isset($_SESSION['barcode-action']) ? $_SESSION['barcode-action'] : "locate";
?>";

var g_preferred_urls = <?php echo json_encode(preferred_urls(/*use_https=*/true),
                                              JSON_HEX_TAG | JSON_HEX_AMP | JSON_PRETTY_PRINT); ?>;

function set_checkin_table_height() {
  $("#main-checkin-table-div").height(
      $(window).height() - $(".banner").height() - $("#top-buttons").height());
}
$(function() { set_checkin_table_height(); });
$(window).on('resize', set_checkin_table_height);
</script>
<script type="text/javascript" src="js/mobile.js"></script>
<script type="text/javascript" src="js/dashboard-ajax.js"></script>
<script type="text/javascript" src="js/modal.js"></script>
<script type="text/javascript" src="js/dropzone.min.js"></script>
<script type="text/javascript" src="js/partitions-modal.js"></script>
<script type="text/javascript" src="js/video-device-picker.js"></script>
<script type="text/javascript" src="js/imagecapture.js"></script>
<script type="text/javascript" src="js/photo-capture-modal.js"></script>
<script type="text/javascript" src="js/checkin.js"></script>
</head>
<body>
<?php
make_banner('Racer Check-In');
?>

<div id="top-buttons" class="block_buttons">
  <img id="barcode-button" src="img/barcode.png"
      onclick="handle_barcode_button_click()"/>
  <input id="mobile-button" type="button" value="Mobile"
      onclick="handle_qrcode_button_click()"/>

  <input class="bulk_button"
        type="button" value="Bulk"
        onclick='show_bulk_form();'/>
<?php if (have_permission(REGISTER_NEW_RACER_PERMISSION)) { ?>
      <input type="button" value="New Racer"
        onclick='show_new_racer_form();'/>
<?php } else { ?>
          <div style='padding: 10px 15px; font-size: x-large; line-height: 1.3; margin-bottom: 20px; margin-top: 3px; '>&nbsp;</div>
<?php } ?>
</div>

<div id="main-checkin-table-div">
<table id="main-checkin-table" class="main_table">
<thead>
  <tr>
    <th/>
    <th><?php
      echo column_header(htmlspecialchars(partition_label(), ENT_QUOTES, 'UTF-8'), 'partition');
    ?></th>
    <th><?php echo column_header('Car Number', 'car'); ?></th>
    <th>Photo</th>
    <th><?php echo column_header('Last Name', 'name'); ?></th>
    <th>First Name</th>
    <th>Car Name &amp; From</th>
    <th>Passed?</th>
    <?php if ($xbs) {
        echo '<th>'.$xbs_award_name.'</th>';
    } ?>
  </tr>
</thead>

<tbody id="main_tbody">

</tbody>
</table>
</div>

<?php

  $stmt = $db->query('SELECT partitionid, name,'
                     .'  (SELECT COUNT(*) FROM RegistrationInfo'
                     .'     WHERE RegistrationInfo.partitionid = Partitions.partitionid) AS count'
                     .' FROM Partitions'
                     .' ORDER BY sortorder');
  $partitions = $stmt->fetchAll(PDO::FETCH_ASSOC);
  if (count($partitions) == 0) {
    // 0 won't be used as a partitionid, but will cause the server to create a new
    // partition with default name.
    $partitions[] = array('partitionid' => 0,
                          'name' => DEFAULT_PARTITION_NAME);
  }


  list($classes, $classseq, $ranks, $rankseq) = classes_and_ranks();

$sql = checkin_table_SELECT_FROM_sql()
    .' ORDER BY '
          .($order == 'car' ? 'carnumber, lastname, firstname' :
            ($order == 'class'  ? 'class_sort, rank_sort, lastname, firstname' :
             ($order == 'partition' ? 'partition_sortorder, lastname, firstname' :
              'lastname, firstname')));

$stmt = $db->prepare($sql);
$stmt->execute(array(':xbs_award_name' => xbs_award()));
?>

<script>
function addrow0(racer) {
  return add_table_row('#main_tbody', racer,
                <?php echo $xbs ? json_encode($xbs_award_name) : "false"; ?>);
}

  <?php
$n = 1;
foreach ($stmt as $rs) {
  // TODO
  $rs['rankseq'] = $ranks[$rs['rankid']]['seq'];
  if (is_null($rs['note'])) {
    $rs['note'] = '';
  }
  echo "addrow0(".json_encode(json_table_row($rs, $n),
                              JSON_NUMERIC_CHECK | JSON_UNESCAPED_SLASHES |
                              JSON_HEX_AMP | JSON_HEX_TAG | JSON_HEX_APOS).");\n";
  ++$n;
}
?>

$(function () {
var partitions = <?php echo json_encode($partitions,
                                        JSON_NUMERIC_CHECK | JSON_UNESCAPED_SLASHES |
                                        JSON_HEX_AMP | JSON_HEX_TAG | JSON_HEX_APOS); ?>;
var partition_label_pl = <?php echo json_encode(partition_label_pl(),
                                        JSON_NUMERIC_CHECK | JSON_UNESCAPED_SLASHES |
                                        JSON_HEX_AMP | JSON_HEX_TAG | JSON_HEX_APOS); ?>;

$("#edit_partition").empty();
for (var i in partitions) {
  var opt = $("<option/>")
      .attr('value', partitions[i].partitionid)
      .text(partitions[i].name);
  opt.appendTo("#edit_partition");
  opt.clone().appendTo("#bulk_who");
}
var opt = $("<option/>")
    .attr('value', -1)
.text("(Edit " + partition_label_pl + ")");
opt.appendTo("#edit_partition");
opt.clone().appendTo("#bulk_who");

mobile_select_refresh($("#edit_partition"));
mobile_select_refresh($("#bulk_who"));

{
  var reorder_modal = PartitionsModal(
    "<?php echo htmlspecialchars(partition_label(), ENT_QUOTES, 'UTF-8'); ?>",
    "<?php echo htmlspecialchars(partition_label_pl(), ENT_QUOTES, 'UTF-8'); ?>",
    partitions, callback_after_partition_modal);

  $("#edit_partition").on('change', function(ev) { on_edit_partition_change(ev.target, reorder_modal); });
  $("#bulk_who").on('change', function(ev) { on_edit_partition_change(ev.target, reorder_modal); });
}

});
</script>


<div id='edit_racer_modal' class="modal_dialog hidden block_buttons">
  <form id="editracerform">
    <div id="left-edit">
      <label for="edit_firstname">First name:</label>
      <input id="edit_firstname" type="text" name="edit_firstname" value=""/>
      <label for="edit_lastname">Last name:</label>
      <input id="edit_lastname" type="text" name="edit_lastname" value=""/>

      <label for="edit_carno">Car number:</label>
      <input id="edit_carno" type="text" name="edit_carno" value=""/>

      <label for="edit_carname">Car name:</label>
      <input id="edit_carname" type="text" name="edit_carname" value=""/>
    </div>

    <div id="right-edit">
      <label for="edit_partition">
        <?php echo htmlspecialchars(partition_label().':', ENT_QUOTES, 'UTF-8'); ?>
      </label>
      <!-- Populated by javascript -->
      <select id="edit_partition" data-wrapper-class="partition_mselect"></select>

      <label for="edit_note_from">From (if desired):</label>
      <input id="edit_note_from" type="text" name="edit_note_from" value=""/>

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
    </div>

  <input id="edit_racer" type="hidden" name="racer" value=""/>

</form>
</div>

<div id='photo_modal' class="modal_dialog hidden block_buttons">
  <form id="photo_drop" class="dropzone">
    <input type="hidden" name="action" value="photo.upload"/>
    <input type="hidden" id="photo_modal_repo" name="repo"/>
    <input type="hidden" id="photo_modal_racerid" name="racerid"/>
    <input type="hidden" name="MAX_FILE_SIZE" value="30000000" />

    <h3>Capture <span id="racer_photo_repo"></span>
        photo for <span id="racer_photo_name"></span>
    </h3>

    <video id="preview" autoplay="true" muted="true" playsinline="true"></video>

    <div id="left-photo">

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
        <input type="button" value="Cancel"
          onclick='close_photo_modal();'/>
    </div>
    </div>
    <div id="right-photo">
        <select id="device-picker"></select>

        <label id="autocrop-label" for="autocrop">Auto-crop after upload:</label>
        <div class="centered_flipswitch">
          <input type="checkbox" class="flipswitch" name="autocrop" id="autocrop" checked="checked"/>
        </div>
<div>
      <a id="thumb-link" class="button_link">To Photo Page</a>
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
  <input type="button" value="Bulk Renumbering"
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
        <!-- Replaced by javascript -->
      </select>

      <div id="numbering_controls" class="hidable">
        <label for="number_auto">Numbering:</label>
        <input id="number_auto" name="number_auto"
               type="checkbox" class="flipswitch eligible-flip"
               checked="checked"
               data-wrapper-class="trophy-eligible-flipswitch"
               data-off-text="Custom"
               data-on-text="Standard"/>
        <?php
            list($car_numbering_mult, $car_numbering_smallest) = read_car_numbering_values();
        ?>
        <div id="numbering_start_div" style="display: none">
          <label for="bulk_numbering_start">Custom numbering from:</label>
          <input type="number" id="bulk_numbering_start" name="bulk_numbering_start"
                                                            disabled="disabled"
                 value="<?php echo $car_numbering_smallest; ?>"/>
        </div>
        <div id="bulk_numbering_explanation">
           <p>Car numbers start at <?php echo $car_numbering_smallest; ?><?php
              if ($car_numbering_mult != 0) { ?><br/>
                  and the hundreds place increments for each <?php echo partition_label_lc(); ?>.
              <?php }
              else {
                echo ".";
              } ?>
           </p>
       </div>

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
        onclick='pop_modal("#bulk_details_modal");'/>
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

<div id="qrcode_settings_modal" class="modal_dialog wide_modal hidden block_buttons">
  <form id="mobile-checkin-form">
    <h2>Mobile Check-In</h2>
    <div id="mobile-checkin-qrcode" style="width: 256px; margin-left: 122px;"></div>
    <div id="mobile-checkin-title" style="text-align: center; font-size: 1.5em; font-weight: bold; margin-bottom: 25px; margin-top: 10px;"></div>
    <input id="mobile-checkin-url" name="mobile-checkin-url" type="text" />

    <a id="mcheckin-link" class="button_link" href="mcheckin.php">Mobile Check-In &gt;</a>
    <input type="button" value="Close" onclick="close_modal('#qrcode_settings_modal');"/>
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
