<?php

// $_POST['queueid']

if (have_permission(SET_UP_PERMISSION)) {
  $stmt = $db->prepare('DELETE FROM Playlist WHERE queueid = :queueid');
  $stmt->execute(array(':queueid' => $_POST['queueid']));

  json_success();
} else {
  json_not_authorized();
}

?>