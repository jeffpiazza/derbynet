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
  reorder_partitions();
  json_success();
} else {
  json_not_authorized();
}
?>