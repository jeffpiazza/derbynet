<?php
// $_POST['queueid_1'] = (first queue entry), ...

if (have_permission(SET_UP_PERMISSION)) {
  foreach ($_POST as $key => $val) {
    if (substr($key, 0, strlen('queueid_')) == 'queueid_') {
      $stmt = $db->prepare('UPDATE Playlist SET seq = :seq WHERE queueid = :queueid');
      $stmt->execute(array(':seq' => substr($key, strlen('queueid_')),
                           ':queueid' => $val));
    }
  }
  json_success();
} else {
  json_not_authorized();
}
?>