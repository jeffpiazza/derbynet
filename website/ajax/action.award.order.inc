<?php

// $_POST['awardid_<n>'], <n> starting at 1.  Value is an awardid in each case
// <n> is the sequential order, e.g. the value of $_POST['awardid_1'] is the awardid which
// should have sort=1

start_response();

if (have_permission(SET_UP_PERMISSION)) {
  foreach ($_POST as $key => $val) {
    if (substr($key, 0, strlen('awardid_')) == 'awardid_') {
      $stmt = $db->prepare('UPDATE Awards SET sort = :sort WHERE awardid = :awardid');
      $stmt->execute(array(':sort' => substr($key, strlen('awardid_')),
                           ':awardid' => $val));
    }
  }
  echo "<success/>\n";
  require('ajax/query.award.list.inc');
} else {
  echo "<failure code='permission'>Not authorized</failure>\n";
}

end_response();
?>