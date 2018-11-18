<?php
session_start();
// This page contains a table of all the "standings" entries, as produced by the
// final_standings() function.  There's a control at the top that lets the user
// choose to view standings by individual rounds, or for the final standings of
// the pack.
//
// If Grand Final rounds are used, the "All" settings is exactly equivalent to
// viewing the results for the last Grand Final round.
//
// TODO We'd like to be able to see how the members of a Grand Final round were
// selected, and what the pack standings were/would have been without the GF
// round.  Note that the roster.new action performs SQL queries directly,
// without using final_standings(), but performs a similar query.
require_once('inc/data.inc');
require_once('inc/banner.inc');
require_once('inc/authorize.inc');
require_once('inc/schema_version.inc');
require_permission(VIEW_RACE_RESULTS_PERMISSION);
require_once('inc/standings.inc');
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/mobile-init.js"></script>
<script type="text/javascript" src="js/jquery.mobile-1.4.2.min.js"></script>
<script type="text/javascript" src="js/standings.js"></script>
<script type="text/javascript">
// This bit of javascript has to be here and not standings.js because of the PHP portions
$(function () {
    // We're initially displaying the "All" case.
    $("tr").not(".headers").addClass('hidden');
    $(select_standings(false, false, <?php echo json_encode(supergroup_label()); ?>)).removeClass('hidden');

    $("select").on("change", function(event) {
        standings_select_on_change($(this).find("option:selected"),
                                   <?php echo json_encode(supergroup_label()); ?>);
      });
});
</script>
<title>Standings</title>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<link rel="stylesheet" type="text/css" href="css/main-table.css"/>
<style type="text/css">
.download_div {
  float: right;
  margin-right: 10px;
}

.center-select {
  width: 400px;
  margin-left: auto;
  margin-right: auto;
}
.center-select h3 {
  text-align: center;
}
</style>
<?php require('inc/stylesheet.inc'); ?>
</head>
<body>
<?php make_banner('Race Standings'); ?>
<div class="block_buttons">

<div class="center-select">
<h3><?php
echo read_raceinfo_boolean('use-points') ? "Scoring by points"
: (read_raceinfo_boolean('drop-slowest') ? "Dropping each racer's slowest time"
                                         : "Averaging all heat times");
?></h3>
</div>

<div class="download_div">
  <a id="download-button" class='button_link' href='export-standings.php'>Download</a>
</div>

<div class="center-select">
<select>
    <option selected="selected">All</option>
    <?php
    $use_subgroups = read_raceinfo_boolean('use-subgroups');
    $rounds = rounds_for_standings();
    foreach ($rounds as $round) {
      echo '<option data-roundid="'.$round['roundid'].'">'
          .htmlspecialchars($round['name'], ENT_QUOTES, 'UTF-8')
          .'</option>'."\n";
      if ($use_subgroups) {
        foreach ($round['ranks'] as $rank) {
          echo '<option data-roundid="'.$round['roundid'].'" data-rankid="'.$rank['rankid'].'">';
          echo htmlspecialchars($round['name'].' / '.$rank['name'], ENT_QUOTES, 'UTF-8');
          echo "</option>\n";
        }
      }
    }
    ?>
</select>
</div>
</div>
<table class="main_table">
<?php
write_standings_table_headers();
$standings = final_standings();
write_standings_table_rows($standings);
?>
</table>
</body>
</html>
