<?php
// Adding a class:
// $_POST['name']
// Optional $_POST['constituent_' + classid] for each constituent classid

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
      $constituents = array();
      foreach ($_POST as $key => $value) {
        if (substr($key, 0, strlen('constituent_')) == 'constituent_') {
          $constituents[] = substr($key, strlen('constituent_'));
        }
      }
      if (empty($constituents)) {  // Normal class
        $classid = create_class($classname);
        // Manufacture a "generic" rank name for this class.  It isn't really
        // well-defined to be creating a class without a rank if we're actually
        // using rank names.
        $rankid = find_or_create_rank($classname, $classid);
        echo "<success/>\n";
      } else {  // Aggregate class
        $classid = create_class($classname, /*with_first_round=*/false);
        $stmt = $db->prepare('UPDATE Classes'
                             .' SET constituents = :constituents,'
                             .' durable = 1'
                             .' WHERE classid = :classid');
        $stmt->execute(array(':constituents' => implode(',', $constituents),
                             ':classid' => $classid));
        echo "<success/>\n";
      }
    }
  } else {
    echo "<failure code='missing-args'>Unrecognized argument signature</failure>\n";
  }
} else {
  echo "<failure code='permission'>Not authorized</failure>\n";
}

end_response();
?>