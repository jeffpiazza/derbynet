<?php
// Adding a class:
// $_POST['name']

require_once('inc/schema_version.inc');
require_once('inc/sql-script.inc');
require_once(sql_file_path('column-widths'));
require_once('inc/newracer.inc');

start_response();

if (have_permission(SET_UP_PERMISSION)) {
  if (isset($_POST['name'])) {
    $classname = substr(trim($_POST['name']), 0, CLASS_COLUMN_WIDTH);
    if ($classname == '') {
      echo "<failure code='noname'>No name provided for the new ".group_label()."</failure>\n";
    } else {
      $classid = create_class($classname);
      // Manufacture a "generic" rank name for this class.  It isn't really
      // well-defined to be creating a class without a rank if we're actually
      // using rank names.
      $rankid = find_or_create_rank($classname, $classid);
      echo "<success/>\n";
    }
  } else {
    echo "<failure code='missing-args'>Unrecognized argument signature</failure>\n";
  }
} else {
  echo "<failure code='permission'>Not authorized</failure>\n";
}

end_response();
?>