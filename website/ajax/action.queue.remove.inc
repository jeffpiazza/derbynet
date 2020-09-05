<?php

// $_POST['queueid']

start_response();

if (have_permission(SET_UP_PERMISSION)) {
  $stmt = $db->prepare('DELETE FROM RacingQueue WHERE queueid = :queueid');
  $stmt->execute(array(':queueid' => $_POST['queueid']));

  echo "<success/>\n";
} else {
  not_authorized_body();
}

end_response();

?>