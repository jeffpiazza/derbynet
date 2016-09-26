<?php @session_start();
require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_permission(SET_UP_PERMISSION);

// TODO Button still says "Import Roster"
// TODO Failure needs some explanation
// TODO: Wipe out existing data (option)

require_once('inc/import-csv.inc');

generate_import_page(
  'Import Awards', 'Import Awards',
  array('js/import-awards.js'),
  /* show_encodings */true,
  array(
    'awardname' => array('name' => "Award Name",
                        'required' => true),
    'awardtype' => array('name' => "Award Type",
                         'required' => true),
    'classname' => array('name' => group_label(),
                         'required' => false),
    'subgroup' => array('name' => subgroup_label(),
                        'required' => false),
    'carnumber' => array('name' => "Car Number",
                         'required' => false)),
  '');
?>
