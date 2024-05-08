<?php @session_start();
require_once('inc/data.inc');
require_once('inc/authorize.inc');
session_write_close();
require_permission(SET_UP_PERMISSION);

require_once('inc/import-csv.inc');

class ImportResults extends ImportCsvGenerator {
  protected function make_encoding_section() {
?>
  <div id="variations">
    &nbsp;
    <div id="sheet">
    </div>
  </div>
<?php
  }  // No encodings to choose
  protected function make_column_labels($labels) {
// See js/import-results.js for the interpretation on import
// See inc/export-results.inc for the columns produced by export
// See ajax/action.result.import.inc for processing on import
?>
    <div class="fields hidden">
    Expected fields, in order:
<table>
   <tr>
      <td>Group/Class</td>
      <td>Round</td>
      <td>Heat</td>
      <td>Lane</td>
      <td>FirstName</td>
      <td>LastName</td>
      <td>CarNumber</td>
      <td>CarName</td>
      <td>FinishTime</td>
      <td>Scale MPH (Ignored)</td>
      <td>FinishPlace</td>
      <td>Completed</td>
   </tr>
</table>
    </div>
<?php
  }
}
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Import Results</title>
<?php make_head_matter_for_import_page(); ?>
<script type="text/javascript" src="js/import-results.js"></script>
<style>
   div.fields {
     padding-left: 120px;
   }
   .fields td {
     border: 2px blue solid;
   }
</style>
</head>
<body>
<?php make_banner('Import Results', 'setup.php');

  $page_maker = new ImportResults;
  $page_maker->make_import_csv_div('Import Race Results', array());
?>
<?php
  require_once('inc/ajax-pending.inc');
?>
</body>
</html>

