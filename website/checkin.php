<?php @session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/banner.inc');
require_once('inc/authorize.inc');
require_once('inc/schema_version.inc');
require_once('inc/photo-config.inc');
require_permission(CHECK_IN_RACERS_PERMISSION);

// This is the racer check-in page.  It appears as a table of all the
// registered racers, with a checkbox (actually, a "flipswitch",
// thanks to transformations peformed by jquery mobile) for each
// racer.  Clicking on the check-in button invokes some javascript
// that sends an ajax POST request to check-in (or un-check-in) that
// racer.  See checkin.js.

// In addition to the actual check-in, it's possible to change a
// racer's car number from this form, or mark the racer for our
// "exclusively by scout" award.

// Here on the server side, a GET request sends HTML for the whole
// page.  POST requests to make changes to the database are sent to
// action.php, and produce just a small XML document.

// TODO- subgroups explanation

// $use_subgroups, from GPRM settings, tells whether we're using
// "subgroups" within each racing group.
$use_subgroups = read_raceinfo_boolean('use-subgroups');

// Our pack provides an "exclusively by scout" award, based on a
// signed statement from the parent.  Collecting the statement is part
// of the check-in process, so there's provision for a checkbox on the
// check-in form.  For groups that don't do this, $xbs will be false
// (and $xbs_award_name will be blank), and the checkboxes won't be
// shown.
$xbs = read_raceinfo_boolean('xbs-award');
$xbs_award_name = read_raceinfo('xbs-award');

$order = '';
if (isset($_GET['order']))
  $order = $_GET['order'];  // Values are: name, den, car
if (!$order)
    $order = 'name';

// The table of racers can be presented in order by name, car, or
// den (and then by name within the den).  Each sortable column has
// a header which is a link to change the ordering, with the
// exception that the header for the column for ordering currently
// in use is NOT a link (because it wouldn't do anything).
function column_header($text, $o) {
    global $order;

    if ($o == $order)
        return '<b>'.$text.'</b>';
    return '<a href="?order='.$o.'">'.$text.'</a>';
}
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<meta http-equiv="refresh" content="300"/>
<title>Check-In</title>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<link rel="stylesheet" type="text/css" href="css/checkin.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/mobile-init.js"></script>
<script type="text/javascript">
g_order = '<?php echo $order; ?>';
</script>
<script type="text/javascript" src="js/jquery.mobile-1.4.2.min.js"></script>
<script type="text/javascript" src="js/dashboard-ajax.js"></script>
<script type="text/javascript" src="js/modal.js"></script>
<script type="text/javascript" src="js/webcam.js"></script>
<script type="text/javascript" src="js/checkin.js"></script>
</head>
<body>
<?php
make_banner('Racer Check-In');

require_once('inc/checkin-table.inc');
?>

<?php if (have_permission(REGISTER_NEW_RACER_PERMISSION)) { ?>
    <div class="block_buttons">
      <input type="button" value="New Racer" data-enhanced="true"
        onclick='show_new_racer_form();'/>
	</div>
<?php } ?>

<table id="main_checkin_table" class="main_table">
<thead>
  <tr>
    <th/>
    <th><?php echo column_header(group_label(), 'den'); ?></th>
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

    $sql = 'SELECT racerid, carnumber, lastname, firstname, carname, imagefile,'
      .(schema_version() < 2 ? "" : " carphoto,")
      .(schema_version() < 2 ? "class" : "Classes.sortorder").' AS class_sort,'
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
            ($order == 'den'  ? 'class_sort, lastname, firstname' :
             'lastname, firstname'));

$stmt = $db->query($sql);

$n = 1;
foreach ($stmt as $rs) {
  checkin_table_row($rs, $xbs, $use_subgroups, $n);
  ++$n;
}
?>
</tbody>
</table>
<?php if (have_permission(REGISTER_NEW_RACER_PERMISSION)) { ?>
    <div class="block_buttons">
      <input type="button" value="New Racer" data-enhanced="true"
        onclick='show_new_racer_form();'/>
	</div>
<?php } ?>


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

    $rank_options = "";
    $sql = 'SELECT rankid, rank, Ranks.classid, class'
           .' FROM Ranks INNER JOIN Classes'
           .' ON Ranks.classid = Classes.classid'
           .' ORDER BY '
           .(schema_version() >= 2 ? 'Classes.sortorder, ' : '')
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
    <input type="checkbox" data-role="flipswitch" name="eligible" id="eligible"
            data-wrapper-class="trophy-eligible-flipswitch"
            data-off-text="Excluded"
            data-on-text="Eligible"/>
  <br/>
  <input type="submit" data-enhanced="true"/>
  <input type="button" value="Cancel" data-enhanced="true"
    onclick='close_modal("#edit_racer_modal");'/>

</form>
</div>

<div id='photo_modal' class="modal_dialog hidden block_buttons">
  <form>
    <input type="hidden" id="photo_modal_repo" name="repo"/>
    <h3>Capture photo for <span id="racer_photo_name"></span></h3>
    <div id="preview">
        <h2>Does your browser support webcams?</h2>
    </div>

    <?php
      if (headshots()->status() != 'ok') {
        echo '<p class="warning">Check <a href="settings.php">photo directory settings</a> before proceeding!</p>';
      }
    ?>

    <div class="block_buttons">
        <input type="submit" value="Capture &amp; Check In" data-enhanced="true" id="capture_and_check_in"
           onclick='g_check_in = true;'/>
        <br/>
        <input type="submit" value="Capture Only" data-enhanced="true"
          onclick='g_check_in = false;'/>
        <input type="button" value="Cancel" data-enhanced="true"
          onclick='close_photo_modal();'/>

        <label id="autocrop-label" for="autocrop">Auto-crop after upload:</label>
        <div class="centered_flipswitch">
          <input type="checkbox" data-role="flipswitch" name="autocrop" id="autocrop" checked="checked"/>
        </div>
    </div>
  </form>
</div>

<?php require_once('inc/ajax-pending.inc'); ?>
<div id="find-racer">
  <form id="find-racer-form">
    Find Racer:
    <input type="text" id="find-racer-text" name="narrowing-text" data-enhanced="true"/>
    <span id="find-racer-message"><span id="find-racer-index" data-index="1">1</span> of <span id="find-racer-count">0</span></span>
    <img onclick="cancel_find_racer()" src="img/cancel-20.png"/>
  </form>
</div>
</body>
</html>
