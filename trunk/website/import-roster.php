<?php @session_start();
require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_permission(SET_UP_PERMISSION);  // TODO: choose the right permission
// TODO: Wipe out existing data (option)
// TODO: Roster management screen, with "delete racer", "delete all racers".
// TODO if ($_POST) {} : 
?><html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Import Roster</title>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/import-roster.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery-ui-1.10.4.min.js"></script>
<script type="text/javascript" src="js/jquery.ui.touch-punch.min.js"></script>
<script type="text/javascript" src="js/checkin.js"></script>
<script type="text/javascript" src="js/jquery.csv.js"></script>
<script type="text/javascript" src="js/import-roster.js"></script>
</head>
<body>
<?php
$banner_title = 'Import Roster';
require('inc/banner.inc');

// TODO:
//
// <div controls> at top
//    Replace Import button with message
// <div file_target>
// <div meta>
// <div classes/ranks> -- maybe show the new classes/ranks, and require accepting each one before import
// <div target>
//   add a little space above table
// <table>
?>
<div class="import_roster">

<div id="controls">
  <div id="meta"></div>
<!-- TODO: wire up start_over_button -->
  <input type="button" id="start_over_button" value="Start Over"/>
  <div id="import_button_div">
    <input class="hidden" type="button" id="import_button" value="Import Roster"/>
    <div id="submit_message">Please select or drag a file to import.</div>
  </div>
</div>

<div class="file_target">
<input type="file" id="csv_file" name="csv_file"/>
</div>

<div id="new_ranks">
</div>

<div class="fields hidden">
<h3>Drag fields to label the data columns for import.</h3>

<div class="target">
<table>
<tr>
<td data-home="lastname"><div data-field="lastname" class="field required">Last Name</div></td>
<td data-home="firstname"><div data-field="firstname" class="field required">First Name</div></td>
<td data-home="classname"><div data-field="classname" class="field required">Den<!-- TODO --></div></td>
<td data-home="carnumber"><div data-field="carnumber" class="field optional">Car Number</div></td>
<td data-home="carname"><div data-field="carname" class="field optional">Car Name</div></td>
<td data-home="subgroup"><div data-field="subgroup" class="field optional">Subgroup<!-- TODO --></div></td>
</tr>
</table>
</div>

</div>

<table id="csv_content">
</table>

</div>

<?php require_once('inc/ajax-pending.inc'); ?>
</body>
</html>