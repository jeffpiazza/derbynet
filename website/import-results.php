<?php @session_start();
require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_permission(SET_UP_PERMISSION);

require_once('inc/import-csv.inc');

class ImportResults extends ImportCsvGenerator {
  protected function make_encoding_section() {
  }  // No encodings to choose
}
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Import Results</title>
<?php make_head_matter_for_import_page(); ?>
<script type="text/javascript" src="js/import-results.js"></script>
</head>
<body>
<?php
  make_banner('Import Results', 'setup.php');
  $page_maker = new ImportResults;
  $page_maker->make_import_csv_div('Import Race Results', array());
?>
<?php
  require_once('inc/ajax-pending.inc');
?>
</body>
</html>

