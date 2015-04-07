<?php
session_start();
require_once('inc/data.inc');
require_once('inc/authorize.inc');
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
          $("tr[data-final='0']").not(".headers").addClass("hidden");
          $("tr[data-final='1']").removeClass("hidden");
          $("#per-group-label").text(<?php echo json_encode(group_label()); ?>);
        } else {
          $("tr:not([data-roundid='" + selection.attr('data-roundid') + "'])").not(".headers").addClass("hidden");
          $("tr[data-roundid='" + selection.attr('data-roundid') + "']").removeClass("hidden");
          $("#per-group-label").text("Round");
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
                   .' ORDER BY class, round DESC');
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
<tr class="headers"><th>In <?php echo supergroup_label(); ?></th>
    <th>In <span id='per-group-label'><?php echo group_label(); ?></span></th>
    <th><?php echo group_label(); ?></th>
    <th>Car Number</th>
    <th>Name</th>
    <th>Average</th>
    <th>Best</th>
    <th>Worst</th>
</tr>

<?php
$ord = 0;
$by_roundid = array();
foreach (final_standings() as $row) {
  echo "<tr data-roundid='".$row['roundid']."' data-final='".$row['final']."'"
       .($row['final'] ? "" : " class='hidden'").">";
  
  echo "<td>";
  if ($row['final']) { echo ++$ord; }
  echo "</td>";

  echo "<td>";
  if (!isset($by_roundid[$row['roundid']])) {
    $by_roundid[$row['roundid']] = 0;
  }
  echo ++$by_roundid[$row['roundid']];
  echo "</td>";

  echo "<td>".htmlspecialchars($row['class'], ENT_QUOTES, 'UTF-8')."</td>";
  echo "<td>".$row['carnumber']."</td>";
  echo "<td>".htmlspecialchars($row['firstname'].' '.$row['lastname'], ENT_QUOTES, 'UTF-8')."</td>";
  echo "<td>".sprintf('%5.3f', $row['time'])."</td>";
  echo "<td>".sprintf('%5.3f', $row['best'])."</td>";
  echo "<td>".sprintf('%5.3f', $row['worst'])."</td>";
  echo "</tr>\n";
}
?>
</table>
</body>
</html>
