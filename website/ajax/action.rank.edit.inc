<?php
// Renaming a rank:
// $_POST['rankid']
// $_POST['name']

start_response();

if (have_permission(SET_UP_PERMISSION)) {
  if (isset($_POST['rankid']) && isset($_POST['name'])) {
    $stmt = $db->prepare('UPDATE Ranks SET rank = :name WHERE rankid = :rankid');
    $stmt->execute(array(':rankid' => $_POST['rankid'],
                         ':name' => trim($_POST['name'])));
    echo "<success/>\n";
  } else {
    echo "<failure code='missing-args'>Unrecognized argument signature</failure>\n";
  }
} else {
  echo "<failure code='permission'>Not authorized</failure>\n";
}

end_response();
?>