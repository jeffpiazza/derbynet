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
$(function () {
    // We're initially displaying the "All" case.
    $("tr").not(".headers").addClass('hidden');
    $(select_standings(false, '<?php echo supergroup_label(); ?>')).removeClass('hidden');

    $("select").on("change", function(event) {
        $("tr").not(".headers").addClass('hidden');
        var selection = $(this).find("option:selected");
        if (typeof selection.attr('data-roundid') == typeof undefined || selection.attr('data-roundid') === false) {
          $(select_standings(false, '<?php echo supergroup_label(); ?>')).removeClass('hidden');
        } else {
          $(select_standings(selection.attr('data-roundid'), selection.text())).removeClass('hidden');
        }
      });
});
</script>
<title>Standings</title>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
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
<?php require('inc/stylesheet.inc'); ?>
</head>
<body>
<?php $banner_title = 'Race Standings'; require('inc/banner.inc'); ?>
<div class="center-select">
<h3><?php echo read_raceinfo_boolean('drop-slowest') ? "Dropping each racer's slowest time" : "Averaging all heat times"; ?></h3>
<select>
    <option selected="selected">All</option>
    <?php
    foreach (standings_round_names() as $round) {
      echo '<option data-roundid="'.$round['roundid'].'">'
          .htmlspecialchars($round['name'], ENT_QUOTES, 'UTF-8')
          .'</option>'."\n";
    }
    ?>
</select>
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
