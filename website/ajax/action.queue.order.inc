<?php
// $_POST['queueid_1'] = (first queue entry), ...

start_response();

if (have_permission(SET_UP_PERMISSION)) {
  foreach ($_POST as $key => $val) {
    if (substr($key, 0, strlen('queueid_')) == 'queueid_') {
      $stmt = $db->prepare('UPDATE RacingQueue SET seq = :seq WHERE queueid = :queueid');
      $stmt->execute(array(':seq' => substr($key, strlen('queueid_')),
                           ':queueid' => $val));
    }
  }
  echo "<success/>\n";
} else {
  echo "<failure code='permission'>Not authorized</failure>\n";
}

end_response();
?>