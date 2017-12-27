<?php

// $_POST['awardid_<n>'], <n> starting at 1.  Value is an awardid in each case
// <n> is the sequential order, e.g. the value of $_POST['awardid_1'] is the awardid which
// should have sort=1

require_once('inc/awards-xml.inc');

start_response();

if (have_permission(EDIT_AWARDS_PERMISSION)) {
  foreach ($_POST as $key => $val) {
    if (substr($key, 0, strlen('awardid_')) == 'awardid_') {
      $stmt = $db->prepare('UPDATE Awards SET sort = :sort WHERE awardid = :awardid');
      $stmt->execute(array(':sort' => substr($key, strlen('awardid_')),
                           ':awardid' => $val));
    }
  }
  echo "<success/>\n";
  emit_award_list(/* include_ad_hoc */ false);
} else {
  echo "<failure code='permission'>Not authorized</failure>\n";
}

end_response();
?>