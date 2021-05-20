<?php
// $_POST['classid_<n>'], <n> starting at 1.  Value is a classid in each case
// <n> is the sequential order, e.g. the value of $_POST['classid_1'] is the classid which
// should have sortorder=1

if (have_permission(SET_UP_PERMISSION)) {
  foreach ($_POST as $key => $val) {
    if (substr($key, 0, strlen('classid_')) == 'classid_') {
      $stmt = $db->prepare('UPDATE Classes SET sortorder = :sortorder WHERE classid = :classid');
      $stmt->execute(array(':sortorder' => substr($key, strlen('classid_')),
                           ':classid' => $val));
    }
  }
  json_success();
} else {
  json_failure('permission', "Not authorized");
}
?>