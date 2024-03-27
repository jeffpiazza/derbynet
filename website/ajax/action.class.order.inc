<?php
// $_POST['classid_<n>'], <n> starting at 1.  Value is a classid in each case
// <n> is the sequential order, e.g. the value of $_POST['classid_1'] is the classid which
// should have sortorder=1

require_once('inc/partitions.inc');

if (have_permission(SET_UP_PERMISSION)) {
  $stmt = $db->prepare('UPDATE Classes SET sortorder = :sortorder WHERE classid = :classid');
  foreach ($_POST as $key => $val) {
    if (substr($key, 0, strlen('classid_')) == 'classid_') {
      $stmt->execute(array(':sortorder' => substr($key, strlen('classid_')),
                           ':classid' => $val));
    }
  }
  // If the $_POST doesn't account for all the classes (e.g., because a
  // new class was added in a different window), then this could end up
  // assigning the same sortorder more than once.  This cleanup makes sure that
  // doesn't happen.  In most cases, the foreach shouldn't find anything to
  // clean up.
  do {
    $did_cleanup = false;
    foreach ($db->query('SELECT MAX(classid) FROM Classes'
                        .' GROUP BY sortorder'
                        .' HAVING COUNT(*) > 1'
                        .' LIMIT 1')
             as $classid) {
      $db->execute_once('UPDATE Classes SET sortorder = sortorder + 1 WHERE classid >= :classid',
                        array(':classid' => $classid));
      $did_cleanup = true;
    }
  } while ($did_cleanup);

  reorder_partitions();
  record_action_partition_structure();
  json_success();
} else {
  json_failure('permission', "Not authorized");
}
?>
