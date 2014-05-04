<?php @session_start(); ?>
<?php
require_once('data.inc');
require_once('authorize.inc');
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
// checkin-action.php, and produce just a small XML document.

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
<link rel="stylesheet" type="text/css" href="jquery-mobile/jquery.mobile-1.4.2.css"/>
<?php require('stylesheet.inc'); ?>
<meta http-equiv="refresh" content="300"/>
<script type="text/javascript" src="jquery.js"></script>
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
</script>
<script type="text/javascript" src="jquery-mobile/jquery.mobile-1.4.2.js"></script>
<script type="text/javascript" src="checkin.js"></script>
</head>
<body>
<?php
$banner_title = 'Racer Check-In';
require('banner.inc');

// This is a hidden form for renumbering a racer's car number.
?>
<div class="block_buttons">
<form id="renumberform" class="floatform hidden" method="POST" onsubmit="handle_renumber(); return false;">
  <label for="renumber_carno">Change car number for
    <span id="renumber_racerprompt">Missing</span>:
  </label>
  <input id="renumber_racer" type="hidden" name="racer" value=""/>
  <input id="renumber_carno" type="text" name="value" value=""/>
  <br/>
  <input type="submit"/>
  <input type="button" value="Cancel"
    onclick='$("#renumberform").addClass("hidden");'/>
</form>
</div>

<div class="block_buttons">
<form id="rankchangeform" class="floatform hidden" method="POST"
	  onsubmit="handle_rank_change(); return false;">
  <label for="rank_picker">Change racing group for<br/>
    <span id="rank_racerprompt">Missing</span>:
  </label>
  <input id="rank_racer" type="hidden" name="racer" value=""/>

  <select id="rank_picker"><?php
    $sql = 'SELECT RankID, Rank, Ranks.ClassID, Class'
           .' from Ranks INNER JOIN Classes'
           .' ON Ranks.ClassID = Classes.ClassID'
           .' order by Class, Rank';
    $rs = odbc_exec($conn, $sql);
    while (odbc_fetch_row($rs)) {
      echo "\n".'<option value="'.odbc_result($rs, 'RankID').'">'
		.htmlspecialchars(odbc_result($rs, 'Class'))
		.' / '.htmlspecialchars(odbc_result($rs, 'Rank'))
	   .'</option>';
    }
  ?>
  </select>
  <br/>
  <input type="submit"/>
  <input type="button" value="Cancel"
    onclick='$("#rankchangeform").addClass("hidden");'/>
</form>
</div>

<table class="main_table">

<?php
$sql = 'SELECT RacerID, CarNumber, LastName, FirstName,'
  .' RegistrationInfo.ClassID, Class, RegistrationInfo.RankID, Rank, PassedInspection, Exclude,'
  .' EXISTS(SELECT 1 FROM RaceChart WHERE RaceChart.RacerID = RegistrationInfo.RacerID) AS Scheduled,'
  .' EXISTS(SELECT 1 FROM RaceChart WHERE RaceChart.ClassID = RegistrationInfo.ClassID) AS DenScheduled,'
  .' EXISTS(SELECT 1 FROM Awards WHERE Awards.AwardName = \''.addslashes($xbs_award_name).'\' AND'
  .'                                   Awards.RacerID = RegistrationInfo.RacerID) AS xbs'
  .' FROM (Ranks'
  .' INNER JOIN (Classes'
  .' INNER JOIN RegistrationInfo'
  .' ON RegistrationInfo.ClassID = Classes.ClassID)'
  .' ON RegistrationInfo.RankID = Ranks.RankID'
  .' AND RegistrationInfo.ClassID = Ranks.ClassID)'
  .' ORDER BY '
  .($order == 'car' ? 'CarNumber, LastName, FirstName' :
    ($order == 'den'  ? 'Class, LastName, FirstName' :
	 'LastName, FirstName'));

$rs = odbc_exec($conn, $sql);

echo '<tr>'
  .'<th>'.column_header(group_label(), 'den').'</th>'
  .($use_subgroups ? '<th>'.subgroup_label().'</th>' : '')
  .'<th>'.column_header('Car Number', 'car').'</th>'
  .'<th>'.column_header('Last Name', 'name').'</th>'
  .'<th>First Name</th>'
  .'<th>Passed?</th>'
  //.'<th>Exclude</th>'
  .($xbs ? '<th>'.$xbs_award_name.'</th>' : '')
  .'</tr>';

$n = 1;
while (odbc_fetch_row($rs)) {
  $racer_id = odbc_result($rs, 'RacerID');
  $passed = odbc_result($rs, 'PassedInspection');
  $dsched = odbc_result($rs, 'DenScheduled');
  $first_name = odbc_result($rs, 'FirstName');
  $last_name = odbc_result($rs, 'LastName');
  
  echo '<tr class="d'.($n & 1).($dsched ? ' dsched' : '').'">';

  echo '<td';
  if (have_permission(CHANGE_RACER_RANK_PERMISSION)) {
    echo ' class="clickable"'
      .' onclick=\'show_rank_change_form("'.$first_name.' '.$last_name.'", '
                    .$racer_id.', '.odbc_result($rs, 'RankID').', this);\'';
  }
  echo '>'.odbc_result($rs, 'Class').'</td>';

  if ($use_subgroups)
    echo '<td>'.odbc_result($rs, 'Rank').'</td>';

  echo '<td id="renumber'.$racer_id.'"';
  if (have_permission(RENUMBER_CAR_PERMISSION)) {
    echo ' class="clickable"'
        .' onclick=\'show_renumber_form("'.$first_name.' '.$last_name.'", '.$racer_id.', this);\'';
  }
  echo '>'
        .odbc_result($rs, 'CarNumber')
        .'</td>';

  echo '<td>'.$last_name.'</td>'
      .'<td>'.$first_name.'</td>';
    // If a race schedule exists for this racer, don't offer the option to un-pass through this interface.
    // Instead, have go through the native GPRM interface, which may require regenerating schedules, etc.

  if (odbc_result($rs, 'Scheduled')) {
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
    //.'<td>'.odbc_result($rs, 'Exclude').'</td>'
    if ($xbs) {
        echo '<td>'
		  .'<label for="xbs-'.$racer_id.'">Exclusively By Scout?</label>'
		  .'<input type="checkbox" data-role="flipswitch" name="xbs-'.$racer_id.'" '
                      .(odbc_result($rs, 'xbs') ? ' checked="checked"' : '')
		              .' data-on-text="Yes" data-off-text="No"'
                      .' onchange="handlechange_xbs(this);"/>'
	    .'</td>';
	}
    echo '</tr>'."\n";
  ++$n;
}

odbc_close($conn);
?>
</table>
<?php if (have_permission(REGISTER_NEW_RACER_PERMISSION)) { ?>
    <div class="block_buttons">
	 <input data-enhanced="true" type="button" value="New Racer" onclick="window.location = 'newracer.php';"/>
	</div>
<?php } ?>
<div id="ajax_working" class="hidden">
  <span id="ajax_num_requests">0</span> request(s) pending.
</div>
</body>
</html>
