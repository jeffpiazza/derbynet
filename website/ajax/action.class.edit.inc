<?php
// Renaming a class:
// $_POST['classid']
// $_POST['name']
// $_POST['ntrophies']

require_once('inc/classes.inc');

start_response();

if (have_permission(SET_UP_PERMISSION)) {
  if (isset($_POST['classid']) && isset($_POST['name'])) {
    $stmt = $db->prepare('UPDATE Classes SET class = :name WHERE classid = :classid');
    $stmt->execute(array(':classid' => $_POST['classid'],
                         ':name' => trim($_POST['name'])));
    if (isset($_POST['ntrophies'])) {
      update_ntrophies($_POST['classid'], $_POST['ntrophies']);
    }
    echo "<success/>\n";
  } else {
    echo "<failure code='missing-args'>Unrecognized argument signature</failure>\n";
  }
} else {
  echo "<failure code='permission'>Not authorized</failure>\n";
}

end_response();
?>