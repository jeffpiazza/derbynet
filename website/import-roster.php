<?php @session_start();
require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_permission(SET_UP_PERMISSION);
// TODO: Wipe out existing data (option)
// TODO: "delete racer", "delete all racers" (as super-powers on checkin page)

require_once('inc/import-csv.inc');

generate_import_page(
  'Import Roster', 'Import Roster',
  array('js/import-roster.js'),
  /* show_encodings */true,
  array(
    'lastname' => array('name' => "Last Name",
                        'required' => true),
    'firstname' => array('name' => "First Name",
                         'required' => true),
    'classname' => array('name' => group_label(),
                         'required' => true),
    'carnumber' => array('name' => "Car Number",
                         'required' => false),
    'carname' => array('name' => "Car Name",
                       'required' => false),
    'subgroup' => array('name' => subgroup_label(),
                        'required' => false)),
  '<div class="footer">'
  .'Or instead: <a href="import-results.php">Import results exported from another race...</a>'
  .'</div>');
?>
