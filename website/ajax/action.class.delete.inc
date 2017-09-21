<?php
if (have_permission(SET_UP_PERMISSION)) {
  start_response();
  try {
    $stmt = $db->prepare('DELETE FROM Classes WHERE classid = :classid');
    if ($stmt->execute(array(':classid' => $_POST['classid']))) {
      echo "<success/>\n";
      require('ajax/query.class.list.inc');
    } else {
      echo "<failure/>\n";
    }
  } catch (Exception $e) {
    echo "<failure code='sql'>".htmlspecialchars($e->getMessage(), ENT_QUOTES, 'UTF-8')."</failure>\n";
  }

  end_response();
} else {
  not_authorized();
}
?>