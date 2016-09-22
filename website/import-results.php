<?php @session_start();
require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_permission(SET_UP_PERMISSION);

require_once('inc/import-csv.inc');

generate_import_page(
  'Import Results',
  array('js/import-results.js'),
  /* show_encodings */false,
  array(),
  '<div id="import_results_button_div" class="hidden">'
  .'<input type="button" id="import_results_button" data-enhanced="true" value="Import Race Results"/>'
  .'</div>');
?>
