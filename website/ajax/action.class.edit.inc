<?php
// Renaming a class:
// $_POST['classid']
// $_POST['name']

start_response();

if (isset($_POST['classid']) && isset($_POST['name'])) {
  $stmt = $db->prepare('UPDATE Classes SET class = :name WHERE classid = :classid');
  $stmt->execute(array(':classid' => $_POST['classid'],
                       ':name' => trim($_POST['name'])));
  echo "<success/>\n";
} else {
  echo "<failure code='missing-args'>Unrecognized argument signature</failure>\n";
}

end_response();
?>