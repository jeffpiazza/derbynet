<?php
// $_POST['rankid_<n>'], <n> starting at 1.  Value is a rankid in each case
// <n> is the sequential order, e.g. the value of $_POST['rankid_1'] is the rankid which
// should have sortorder=1

require_once('inc/partitions.inc');

if (have_permission(SET_UP_PERMISSION)) {
  foreach ($_POST as $key => $val) {
    if (substr($key, 0, strlen('rankid_')) == 'rankid_') {
      $stmt = $db->prepare('UPDATE Ranks SET sortorder = :sortorder WHERE rankid = :rankid');
      $stmt->execute(array(':sortorder' => substr($key, strlen('rankid_')),
                           ':rankid' => $val));
    }
  }
  // If the $_POST doesn't account for all the ranks in the class, then this
  // could end up assigning the same sortorder more than once.  This cleanup
  // makes sure that doesn't happen.  In most cases, the foreach shouldn't find
  // anything to clean up.
  do {
    $did_cleanup = false;
    foreach ($db->query('SELECT classid, MAX(rankid) AS rankid FROM Ranks'
                        .' GROUP BY classid, sortorder'
                        .' HAVING COUNT(*) > 1'
                        .' LIMIT 1')
             as $row) {
      $db->execute_once('UPDATE Ranks SET sortorder = sortorder + 1'
                        .' WHERE classid = :classid AND rankid >= :rankid',
                        array(':classid' => $row['classid'],
                              ':rankid' => $row['rankid']));
      $did_cleanup = true;
    }
  } while ($did_cleanup);

  reorder_partitions();
  json_success();
} else {
  json_not_authorized();
}
?>
