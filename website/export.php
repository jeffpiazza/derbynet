<?php @session_start();

require_once('inc/banner.inc');

require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_once('inc/name-mangler.inc');
require_once('inc/awards.inc');

require_once('inc/export-roster.inc');
require_once('inc/export-results.inc');
require_once('inc/export-standings.inc');
require_once('inc/export-awards.inc');


$workbook = array();

$roster = array();
export_roster(function($row) { global $roster; $roster[] = $row; });
$workbook[] = array('Roster', $roster);

$results = array();
export_results(function($row) { global $results; $results[] = $row; });
$workbook[] = array('Results', $results);


function AddStandings($roundid, $rankid, $title) {
  global $workbook;
  global $standings;
  $standings = array();
  export_standings(function($row) { global $standings; $standings[] = $row; },
                   $roundid, $rankid);
  $workbook[] = array($title, $standings);
}
$use_subgroups = read_raceinfo_boolean('use-subgroups');
$rounds = rounds_for_standings();

if (last_aggregate_roundid() !== 0) {
  AddStandings(false, false, 'Standings');
}
foreach ($rounds as $round) {
  AddStandings($round['roundid'], false, 'Standings '.$round['name']);
  if ($use_subgroups) {
    foreach ($round['ranks'] as $rank) {
      AddStandings($round['roundid'], $rank['rankid'],
                   'Standings '.$round['name'].' / '.$rank['name']);
    }
  }
}


$awards = array();
export_awards(function($row) { global $awards; $awards[] = $row; });
$workbook[] = array('Awards', $awards);

?>
<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Export</title>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<?php require('inc/stylesheet.inc'); ?>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery.ui.touch-punch.min.js"></script>
<script type="text/javascript" src="js/dashboard-ajax.js"></script>
<script type="text/javascript" src="js/xlsx.full.min.js"></script>
<script type="text/javascript" src="js/modal.js"></script>
<script type="text/javascript" src="js/plurals.js"></script>
<script type="text/javascript">

var workbook;

$(function() {
    workbook = XLSX.utils.book_new();
    var wb_json = workbook_json();
    for (var sh in wb_json) {
      var rec = wb_json[sh];  // { title, sheet }
      console.log("Building sheet " + rec[0]);
      XLSX.utils.book_append_sheet(workbook,
                                   XLSX.utils.aoa_to_sheet(rec[1]),
                                   rec[0]);
    }
  });

function write_workbook(extension) {
  XLSX.writeFile(workbook, 'derbynet-<?php echo date('Y-m-d'); ?>.' + extension);
}

</script>
<script type="text/javascript">
function workbook_json() {
   return
   // START_JSON
   <?php echo json_encode($workbook, JSON_HEX_TAG | JSON_HEX_AMP); ?>

   // END_JSON
   ;
}
</script>
</head>

<body>
<?php make_banner('Export Data'); ?>
<div class="block_buttons" style="margin-top: 20px;">
<input type="button" data-enhanced="true" value="As .xlsx" onclick="write_workbook('xlsx');"/>
<input type="button" data-enhanced="true" value="As .ods" onclick="write_workbook('ods');"/>
<input type="button" data-enhanced="true" value="As .xls" onclick="write_workbook('xls');"/>
</div>
</body>
</html>

