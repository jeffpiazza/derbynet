<?php
// $_POST['queueid']

// $_POST['n_times_per_lane']
//
// $_POST['sceneid_at_finish'] or
// $_POST['continue_racing'] or
// nothing (stop)

start_response();

if (have_permission(SET_UP_PERMISSION)) {
  $ok = true;

  if (isset($_POST['n_times_per_lane'])) {
    $n_times_per_lane = $_POST['n_times_per_lane'];
  } else {
    $n_times_per_lane = -1;
  }

  if ($ok && isset($_POST['sceneid_at_finish'])) {
    $sceneid_at_finish = $_POST['sceneid_at_finish'];
  } else {
    $sceneid_at_finish = -1;
  }
  if ($ok && isset($_POST['continue_racing'])) {
    $continue_racing = $_POST['continue_racing'] ? 1 : 0;
  } else {
    $continue_racing = 0;
  }

  $stmt = $db->prepare('UPDATE RacingQueue'
                       .' SET n_times_per_lane = :ntimes,'
                       .'     sceneid_at_finish = :sceneid,'
                       .'     continue_racing = :continue'
                       .' WHERE queueid = :queueid');
  $stmt->execute(array(':queueid' => $_POST['queueid'],
                       ':ntimes' => $n_times_per_lane,
                       ':sceneid' => $sceneid_at_finish,
                       ':continue' => $continue_racing));
  echo "<success/>\n";
} else {
  not_authorized_body();
}

end_response();

?>