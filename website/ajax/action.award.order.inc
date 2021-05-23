<?php

// $_POST['awardid_<n>'], <n> starting at 1.  Value is an awardid in each case
// <n> is the sequential order, e.g. the value of $_POST['awardid_1'] is the awardid which
// should have sort=1

require_once('inc/json-awards.inc');
require_once('inc/json-classes.inc');

if (have_permission(EDIT_AWARDS_PERMISSION)) {
  foreach ($_POST as $key => $val) {
    if (substr($key, 0, strlen('awardid_')) == 'awardid_') {
      $stmt = $db->prepare('UPDATE Awards SET sort = :sort WHERE awardid = :awardid');
      $stmt->execute(array(':sort' => substr($key, strlen('awardid_')),
                           ':awardid' => $val));
    }
  }
  json_success();
  json_out_awards(false);
} else {
  json_not_authorized();
}
?>