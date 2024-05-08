<?php
session_start();
// This page contains a table of all the "standings" entries, as produced by the
// result_summary() function.  There's a control at the top that lets the user
// choose to view standings by individual rounds, or for the final standings of
// the pack.
//
// If rounds are present for a single aggregate class, the "All" settings is
// exactly equivalent to viewing the results for the last round of the aggregate
// class.
//
// TODO We'd like to be able to see how the members of an aggregate class were
// selected, and what the pack standings were/would have been without the agg
// round(s).  Note that the roster.new action performs SQL queries directly,
// without using result_summary(), but performs a similar query.
require_once('inc/data.inc');
require_once('inc/authorize.inc');
session_write_close();
require_once('inc/banner.inc');
require_once('inc/schema_version.inc');
require_permission(VIEW_RACE_RESULTS_PERMISSION);
require_once('inc/standings.inc');
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/mobile.js"></script>
<script type="text/javascript" src="js/standings.js"></script>
<script type="text/javascript">
// This bit of javascript has to be here and not standings.js because of the PHP portions
$(function () {
    $("tr").not(".headers").addClass('hidden');

    $("#view-selector").on("change", function(event) {
        standings_select_on_change($(this).find("option:selected"));
      });

    standings_select_on_change($("#view-selector").find("option:selected"));
});
</script>
<title>Standings</title>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/mobile.css"/>
<link rel="stylesheet" type="text/css" href="css/main-table.css"/>
<style type="text/css">

.center-select {
  width: 400px;
  margin-left: auto;
  margin-right: auto;
}
.center-select h3 {
  text-align: center;
}
</style>
</head>
<body>
<?php make_banner('Race Standings'); ?>
<div class="block_buttons">

<div class="center-select">
<h3><?php
$scoring = read_raceinfo('scoring', 0);
if (read_raceinfo_boolean('use-points')) {
  if ($scoring == 0) {
    echo "Scoring by points";
  } else if ($scoring == 1) {
    echo "Scoring by points,<br/>dropping each racer's worst heat";
  } else {
    echo "Scoring by points,<br/>best heat";
  }
} else {
  if ($scoring == 0) {
    echo "Averaging all heat times";
  } else if ($scoring == 1) {
    echo "Dropping each racer's worst heat";
  } else {
    echo "Selecting best heat";
  }
}
?></h3>
</div>

<?php

$standings = new StandingsOracle();

if (isset($_GET['debug'])) {
  echo "<pre>\n";
  echo json_encode($standings->debug_summary(), JSON_PRETTY_PRINT | JSON_HEX_TAG);
  echo "</pre>\n";
}
?>

<div class="center-select">
<select id="view-selector">
<?php

foreach ($standings->standings_catalog() as $entry) {
  echo "<option data-presentation=\"$entry[presentation]\" value=\"$entry[key]\">"
         .htmlspecialchars($entry['name'], ENT_QUOTES, 'UTF-8')
         ."</option>\n";
}
    ?>
</select>
</div>
</div>

<table class="main_table">
<?php
$standings->write_standings_table();
?>
</table>
</body>
</html>
