<?php
// Adding a class:
// $_POST['name']
// Optional $_POST['ntrophies']
// For aggregate classes, use EITHER constituent_ keys or rankid_ keys, but not both:
//     $_POST['constituent_' + classid] for each constituent classid
//  OR $_POST['rankid_' + rankid] for each constituent rankid

require_once('inc/schema_version.inc');
require_once('inc/sql-script.inc');
require_once(sql_file_path('column-widths'));
require_once('inc/newracer.inc');
require_once('inc/classes.inc');
require_once('inc/aggregate_round.inc');

if (have_permission(SET_UP_PERMISSION)) {
  if (isset($_POST['name'])) {
    $classname = substr(trim($_POST['name']), 0, CLASS_COLUMN_WIDTH);
    if ($classname == '') {
      json_failure('noname', "No name provided for the new ".group_label());
    } else {
      list($classes, $classseq, $ranks, $rankseq) = classes_and_ranks();
      $constituents = array();
      $rankids = array();
      foreach ($_POST as $key => $value) {
        if (substr($key, 0, strlen('constituent_')) == 'constituent_') {
          $constituents[] = substr($key, strlen('constituent_'));
        } else if (substr($key, 0, strlen('rankid_')) == 'rankid_') {
          $r = substr($key, strlen('rankid_'));
          $rankids[] = $r;
          $constituents[] = $ranks[$r]['classid'];
        }
        $constituents = array_unique($constituents);
      }
      if (empty($constituents)) {  // Normal class
        $classid = create_class($classname);
        // Manufacture a "generic" rank name for this class.  It isn't really
        // well-defined to be creating a class without a rank if we're actually
        // using rank names.
        $rankid = find_or_create_rank($classname, $classid);
      } else {  // Aggregate class
        $classid = create_aggregate_class($classname, $constituents, $rankids, /*durable*/1);
      }
      if (isset($_POST['ntrophies'])) {
        update_ntrophies($classid, $_POST['ntrophies']);
      }
      json_success();
    }
  } else {
    json_failure('missing-args', "Unrecognized argument signature");
  }
} else {
  json_failure('permission', "Not authorized");
}
?>