<?php
session_start();
// This page contains a table of all the "standings" entries, as produced by the
// final_standings() function.  There's a control at the top that lets the user
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
    $("tr").not(".headers").addClass('hidden');

    $("select").on("change", function(event) {
        standings_select_on_change($(this).find("option:selected"),
                                   <?php echo json_encode(supergroup_label()); ?>);
      });

    standings_select_on_change($("#view-selector").find("option:selected"),
                               <?php echo json_encode(supergroup_label()); ?>);
});
</script>
<title>Standings</title>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<?php require('inc/stylesheet.inc'); ?>
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
</head>
<body>
<?php make_banner('Race Standings'); ?>
<div class="block_buttons">

<div class="center-select">
<h3><?php
if (read_raceinfo_boolean('use-points')) {
  if (read_raceinfo_boolean('drop-slowest')) {
    echo "Scoring by points,<br/>dropping each racer's worst heat";
  } else {
    echo "Scoring by points";
  }
} else if (read_raceinfo_boolean('drop-slowest')) {
  echo "Dropping each racer's worst heat";
} else {
  echo "Averaging all heat times";
}
?></h3>
</div>

<div class="download_div">
  <a id="download-button" class='button_link' href='export-standings.php'>Download</a>
</div>
<?php
$standings = final_standings();
?>
<div class="center-select">
<select id="view-selector">
    <?php
    if (last_aggregate_roundid() !== 0) {
      echo "<option selected=\"selected\">All</option>\n";
    }
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
write_standings_table_rows($standings);
?>
</table>
</body>
</html>
