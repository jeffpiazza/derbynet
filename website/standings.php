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
// round.  Note that the make-roster action performs SQL queries directly,
// without using final_standings(), but performs a similar query.
//
// TODO View all rounds for native classes' n-th round?
require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_once('inc/schema_version.inc');
require_permission(VIEW_RACE_RESULTS_PERMISSION);
require_once('inc/standings.inc');
?>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery.mobile-1.4.2.min.js"></script>
<script type="text/javascript">
$(function () {
    $("select").on("change", function(event) {
        var selection = $(this).find("option:selected");
        if (typeof selection.attr('data-roundid') == typeof undefined || selection.attr('data-roundid') === false) {
          // 'All' case
          $("tr[data-for-supergroup='0']").not(".headers").addClass("hidden");
          $("tr[data-for-supergroup='1']").removeClass("hidden");
          // $("#per-group-label").text(<?php echo json_encode(group_label()); ?>);
        } else {
          $("tr:not([data-roundid='" + selection.attr('data-roundid') + "'])").not(".headers").addClass("hidden");
          $("tr[data-roundid='" + selection.attr('data-roundid') + "']").removeClass("hidden");
          // $("#per-group-label").text("Round");
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
$stmt = $db->query('SELECT class, round, R1.roundid, R1.classid,'
                   .' (SELECT MAX(round) FROM Rounds R2 WHERE R2.classid = R1.classid) AS max_round'
                   .' FROM Rounds R1'
                   .' INNER JOIN Classes'
                   .' ON R1.classid = Classes.classid'
                   .' ORDER BY '
                   .(schema_version() >= 2 ? 'Classes.sortorder, ' : '')
                   .'class, round DESC');
foreach ($stmt as $row) {
  $t = $row['class'];
  if ($row['round'] < $row['max_round']) {
    $t .= ', Round '.$row['round'];
  }
  echo '<option data-roundid="'.$row['roundid'].'">'.htmlspecialchars($t, ENT_QUOTES, 'UTF-8').'</option>'."\n";
}
?>
</select>
</div>
<table class="main_table">
<tr class="headers">
    <th>Place</th>
    <th>Car Number</th>
    <th>Name</th>
    <th><?php echo group_label(); ?></th>
    <th>In <span id='per-group-label'><?php echo group_label(); ?></span></th>
    <th>In <?php echo supergroup_label(); ?></th>
    <th>Heats</th>
    <th>Average</th>
    <th>Best</th>
    <th>Worst</th>
</tr>

<?php

$standings = final_standings();

$pack_so_far = 0;
// $so_far_by_class[$classid] tells how many racers in that (native) $classid
// we've encountered so far.  1 + $so_far_by_class[$classid] is the next place
// to award in the class (1 = 1st, etc.).
$so_far_by_class = array();
// $racer_class_ranking[$racerid] gives rank within class, 1 = 1st
$racer_class_ranking = array();
// $racer_pack_ranking[$racerid] gives rank within the pack, 1 = 1st
$racer_pack_ranking = array();
foreach ($standings as $row) {
  $racerid = $row['racerid'];
  $classid = $row['classid'];
  if ($row['for_group']) {
    if (!isset($so_far_by_class[$classid])) {
      $so_far_by_class[$classid] = 0;
    }
    ++$so_far_by_class[$classid];
    $racer_class_ranking[$racerid] = $so_far_by_class[$classid];
  }
  if ($row['for_supergroup']) {
    ++$pack_so_far;
    $racer_pack_ranking[$racerid] = $pack_so_far;
  }
}
    
$ord = 0;
$by_roundid = array();
foreach ($standings as $row) {
  $roundid = $row['roundid'];
  $racerid = $row['racerid'];
  echo "<tr data-roundid='".$roundid."' data-for-supergroup='".$row['for_supergroup']."'"
       .($row['for_supergroup'] ? "" : " class='hidden'").">";

  echo "<td>";
  if (!isset($by_roundid[$roundid])) {
    $by_roundid[$roundid] = 0;
  }
  echo ++$by_roundid[$roundid];  // Track order within roundid
  echo "</td>";

  echo "<td>".$row['carnumber']."</td>";
  echo "<td>".htmlspecialchars($row['firstname'].' '.$row['lastname'], ENT_QUOTES, 'UTF-8')."</td>";

  // Class
  echo "<td>".htmlspecialchars($row['class'], ENT_QUOTES, 'UTF-8')."</td>";
  // Place in class
  echo "<td>";
  if (isset($racer_class_ranking[$racerid])) echo $racer_class_ranking[$racerid];
  echo "</td>";
  // Place in pack
  echo "<td>";
  if (isset($racer_pack_ranking[$racerid])) echo $racer_pack_ranking[$racerid];
  echo "</td>";
  
  echo "<td>".$row['base']."</td>";
  echo "<td>".sprintf('%5.3f', $row['time'])."</td>";
  echo "<td>".sprintf('%5.3f', $row['best'])."</td>";
  echo "<td>".sprintf('%5.3f', $row['worst'])."</td>";
  echo "</tr>\n";
}
?>
</table>
</body>
</html>
