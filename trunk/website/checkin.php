<?php @session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_permission(CHECK_IN_RACERS_PERMISSION);

// TODO: EDIT_RACERS_PERMISSION

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
?>
<html>
<head>
<title>Check-In</title>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<link rel="stylesheet" type="text/css" href="css/checkin.css"/>
<?php require('inc/stylesheet.inc'); ?>
<meta http-equiv="refresh" content="300"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript">
// We're using jQuery Mobile for its nice mobile/tablet-friendly UI
// elements.  By default, jQM also wants to hijack page loading
// functionality, and perform page loads via ajax, a "service" we
// don't really want.  Fortunately, it's possible to turn that stuff
// off.
$(document).bind("mobileinit", function() {
				   $.extend($.mobile, {
					 ajaxEnabled: false
					 });
				 });

g_order = '<?php echo $order; ?>';
</script>
<script type="text/javascript" src="js/jquery.mobile-1.4.2.min.js"></script>
<script type="text/javascript" src="js/checkin.js"></script>
</head>
<body>
<?php
$banner_title = 'Racer Check-In';
require('inc/banner.inc');
?>

<table class="main_table">
<thead>
  <tr>
    <th/>
    <th><?php echo column_header(group_label(), 'den'); ?></th>
    <?php if ($use_subgroups) {
        echo '<th>'.subgroup_label().'</th>';
    } ?>
    <th><?php echo column_header('Car Number', 'car'); ?></th>
    <th><?php echo column_header('Last Name', 'name'); ?></th>
    <th>First Name</th>
    <th>Passed?</th>
    <?php if ($xbs) {
        echo '<th>'.$xbs_award_name.'</th>';
    } ?>
  </tr>
</thead>

<tbody>
<?php

$sql = 'SELECT racerid, carnumber, lastname, firstname,'
  .' RegistrationInfo.classid, class, RegistrationInfo.rankid, rank, passedinspection, exclude,'
  .' EXISTS(SELECT 1 FROM RaceChart WHERE RaceChart.racerid = RegistrationInfo.racerid) AS scheduled,'
  .' EXISTS(SELECT 1 FROM RaceChart WHERE RaceChart.classid = RegistrationInfo.classid) AS denscheduled,'
  .' EXISTS(SELECT 1 FROM Awards WHERE Awards.awardname = \''.addslashes($xbs_award_name).'\' AND'
  .'                                   Awards.racerid = RegistrationInfo.racerid) AS xbs'
  .' FROM (Ranks'
  .' INNER JOIN (Classes'
  .' INNER JOIN RegistrationInfo'
  .' ON RegistrationInfo.classid = Classes.classid)'
  .' ON RegistrationInfo.rankid = Ranks.rankid'
  .' AND RegistrationInfo.classid = Ranks.classid)'
  .' ORDER BY '
  .($order == 'car' ? 'carnumber, lastname, firstname' :
    ($order == 'den'  ? 'class, lastname, firstname' :
	 'lastname, firstname'));
$stmt = $db->query($sql);

$n = 1;
foreach ($stmt as $rs) {
  $racer_id = $rs['racerid'];
  $passed = $rs['passedinspection'];
  $dsched = $rs['denscheduled'];
  // TODO: Use of htmlspecialchars should be universal...
  $first_name = htmlspecialchars($rs['firstname'], ENT_QUOTES);
  $last_name = htmlspecialchars($rs['lastname'], ENT_QUOTES);
  
  echo '<tr class="d'.($n & 1).($dsched ? ' dsched' : '').'">';

  echo '<td><input type="button" value="Change" onclick=\'show_edit_racer_form('
      .$racer_id.');\'/></td>';

  echo '<td id="class-'.$racer_id.'" class="sort-class" data-rankid="'.$rs['rankid'].'">'.$rs['class'].'</td>';

  if ($use_subgroups)
    echo '<td id="rank-'.$racer_id.'">'.$rs['rank'].'</td>';

  echo '<td class="sort-car-number" id="car-number-'.$racer_id.'">'.$rs['carnumber'].'</td>';

  echo '<td class="sort-lastname" id="lastname-'.$racer_id.'">'.$last_name.'</td>'
      .'<td class="sort-firstname" id="firstname-'.$racer_id.'">'.$first_name.'</td>';
    // If a race schedule exists for this racer, don't offer the option to un-pass through this interface.
    // Instead, have go through the native GPRM interface, which may require regenerating schedules, etc.

  if ($rs['scheduled']) {
    echo '<td>'.($passed ? 'Racing' : 'Scheduled but not passed').'</td>';
  } else {
      echo '<td>'
		//.'<div data-role="fieldcontain">'
		.'<label for="passed-'.$racer_id.'">Checked In?</label>'
	      .'<input type="checkbox" data-role="flipswitch" name="passed-'.$racer_id.'" id="passed-'.$racer_id.'"'
	          .($passed ? ' checked="checked"' : '')
	          .(($passed && !have_permission(REVERT_CHECK_IN_PERMISSION))
		    ? ' disabled="disabled"' : '')
		.' data-on-text="Yes" data-off-text="No"'
	          .' onchange=\'handlechange_passed(this, "'.$first_name.' '.$last_name.'");\'/>'
	       .($dsched ? ' Late!' : '')
		//.'</div>'
		.'</td>';
  }
    // Racers are normally excluded ahead of time, not as part of the check-in process.
    //.'<td>'.$rs['exclude'].'</td>'
    if ($xbs) {
        echo '<td>'
		  .'<label for="xbs-'.$racer_id.'">Exclusively By Scout?</label>'
		  .'<input type="checkbox" data-role="flipswitch" name="xbs-'.$racer_id.'" '
                      .($rs['xbs'] ? ' checked="checked"' : '')
		              .' data-on-text="Yes" data-off-text="No"'
                      .' onchange="handlechange_xbs(this);"/>'
	    .'</td>';
	}
    echo '</tr>'."\n";
  ++$n;
}
?>
</tbody>
</table>
<?php if (have_permission(REGISTER_NEW_RACER_PERMISSION)) { ?>
    <div class="block_buttons">
	 <input data-enhanced="true" type="button" value="New Racer" onclick="window.location = 'newracer.php';"/>
	</div>
<?php } ?>

<div class="block_buttons">
<?php
// This is a hidden form for editing information about a racer
?>
<form id="editracerform" class="editform hidden" method="POST"
	  onsubmit="handle_edit_racer(); return false;">

  <input id="edit_racer" type="hidden" name="racer" value=""/>

  <label for="edit_firstname">First name:</label>
  <input id="edit_firstname" type="text" name="edit_firstname" value=""/>
  <label for="edit_lastname">First name:</label>
  <input id="edit_lastname" type="text" name="edit_lastname" value=""/>

  <label for="edit_carno">Change car number:</label>
  <input id="edit_carno" type="text" name="edit_carno" value=""/>
  <br/>

  <label for="edit_rank">Change racing group:</label>
  <select id="edit_rank"><?php
    $sql = 'SELECT rankid, rank, Ranks.classid, class'
           .' FROM Ranks INNER JOIN Classes'
           .' ON Ranks.classid = Classes.classid'
           .' ORDER BY class, rank';
    $stmt = $db->query($sql);
    foreach ($stmt as $rs) {
      echo "\n".'<option value="'.$rs['rankid'].'"'
            .' data-class="'.htmlspecialchars($rs['class']).'"'
            .' data-rank="'.htmlspecialchars($rs['rank']).'"'
           .'>'
		.htmlspecialchars($rs['class'])
		.' / '.htmlspecialchars($rs['rank'])
	   .'</option>';
    }
  ?>
  </select>
  <br/>
  <input type="submit" data-enhanced="true"/>
  <input type="button" value="Cancel" data-enhanced="true"
    onclick='$("#editracerform").addClass("hidden");'/>
</form>

</div>

<?php require_once('inc/ajax-pending.inc'); ?>
</body>
</html>
