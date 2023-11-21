<?php @session_start();

require_once('inc/banner.inc');

require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_once('inc/name-mangler.inc');
require_once('inc/awards.inc');
require_once('inc/export-all.inc');

require_permission(VIEW_RACE_RESULTS_PERMISSION);

$workbook = export_all();

?>
<!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Export Results</title>
<?php require('inc/stylesheet.inc'); ?>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery.ui.touch-punch.min.js"></script>
<script type="text/javascript" src="js/dashboard-ajax.js"></script>
<script type="text/javascript" src="js/xlsx.full.min.js"></script>
<script type="text/javascript" src="js/modal.js"></script>
<script type="text/javascript" src="js/plural.js"></script>
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
       echo json_encode($workbook, JSON_HEX_QUOT | JSON_HEX_TAG | JSON_HEX_AMP | JSON_NUMERIC_CHECK); ?>;
   // END_JSON
}
</script>
</head>

<body>
<?php make_banner('Export Results'); ?>
<div class="block_buttons" style="margin-top: 20px;">
<input type="button" value="As .xlsx" onclick="write_workbook('xlsx');"/>
<input type="button" value="As .ods" onclick="write_workbook('ods');"/>
<input type="button" value="As .xls" onclick="write_workbook('xls');"/>
</div>
</body>
</html>
