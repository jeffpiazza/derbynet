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

////////////// Roster
$roster = array();
export_roster(function($row) { global $roster; $roster[] = $row; });
$workbook[] = array('Roster', $roster);


////////////// Results
$results = array();
export_results(function($row) { global $results; $results[] = $row; });
$workbook[] = array('Results', $results);


////////////// Standings
$result_summary = result_summary();
list($finishers, $parallel) = compute_all_standings($result_summary);

function AddStandings(callable $choose, $entry, $title) {
  global $workbook;
  global $result_summary;
  global $parallel;
  $sheet = array();
  $sheet[] = array('Standings for '.$entry['name']);
  export_standings(function($row) use (&$sheet) { $sheet[] = $row; },
                   $choose, $entry, $result_summary, $parallel);
  $workbook[] = array($title, $sheet);
}

$use_subgroups = read_raceinfo_boolean('use-subgroups');

foreach (standings_catalog() as $entry) {
  $name = 'Standings '.preg_replace('+[/]+', '-', $entry['name']);
  // XLSX won't allow sheet name to exceed 31 characters
  if (strlen($name) > 31) {
    $name = 'Standings '.$entry['key'];
  }
  if ($entry['kind'] == 'supergroup') {
    AddStandings(function(&$row, &$p) {
        return isset($p['supergroup']); },
      $entry, 'Standings');
  } else if ($entry['kind'] == 'class' || $entry['kind'] == 'round') {
    AddStandings(function(&$row, &$p) use (&$entry) {
        return $entry['roundid'] == $row['roundid']; },
      $entry, $name);
  } else if ($entry['kind'] == 'rank' || $entry['kind'] == 'rank-round') {
    AddStandings(function(&$row, &$p) use (&$entry) {
        return $entry['roundid'] == $row['roundid'] &&
               $entry['rankid'] == $row['rankid']; },
      $entry, $name);
  } else if ($entry['kind'] == 'agg-class') {
    AddStandings(function(&$row, &$p) use (&$entry) {
        return isset($p[$entry['key']]); },
      $entry, $name);
  }
}


////////////// Awards
$awards = array();
export_awards(function($row) { global $awards; $awards[] = $row; });
$workbook[] = array('Awards', $awards);
?>
<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Export Results</title>
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
    console.log('On page load');
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
   // START_JSON
   return   <?php
       // Tests depend on the export JSON being on one line, so don't use JSON_PRETTY_PRINT
       echo json_encode($workbook, JSON_HEX_QUOT | JSON_HEX_TAG | JSON_HEX_AMP); ?>;
   // END_JSON
}
</script>
</head>

<body>
<?php make_banner('Export Results'); ?>
<div class="block_buttons" style="margin-top: 20px;">
<input type="button" data-enhanced="true" value="As .xlsx" onclick="write_workbook('xlsx');"/>
<input type="button" data-enhanced="true" value="As .ods" onclick="write_workbook('ods');"/>
<input type="button" data-enhanced="true" value="As .xls" onclick="write_workbook('xls');"/>
</div>
</body>
</html>
