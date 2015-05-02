<?php
// $_POST['classid_<n>'], <n> starting at 1.  Value is a classid in each case

start_response();

foreach ($_POST as $key => $val) {
  if (substr($key, 0, strlen('classid_')) == 'classid_') {
    $stmt = $db->prepare('UPDATE Classes SET sortorder = :sortorder WHERE classid = :classid');
    $stmt->execute(array(':sortorder' => substr($key, strlen('classid_')),
                         ':classid' => $val));
  }
}

echo "<success/>\n";

end_response();
?>