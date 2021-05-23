<?php
// $_POST['queueid']

// $_POST['top']
// $_POST['bucketed'] (0 or 1), default 0 if missing
//
// $_POST['n_times_per_lane']
//
// $_POST['sceneid_at_finish'] or
// $_POST['continue_racing'] or
// nothing (stop)

if (have_permission(SET_UP_PERMISSION)) {
  $ok = true;

  $top = $_POST['top'];
  $bucketed = isset($_POST['bucketed']) ? $_POST['bucketed'] : 0;
  $n_times_per_lane = $_POST['n_times_per_lane'];
  $sceneid_at_finish = $_POST['sceneid_at_finish'];
  $continue_racing = $_POST['continue_racing'] ? 1 : 0;

  $stmt = $db->prepare('UPDATE Playlist'
                       .' SET bucket_limit = :top,'
                       .'     bucketed = :bucketed,'
                       .'     n_times_per_lane = :ntimes,'
                       .'     sceneid_at_finish = :sceneid,'
                       .'     continue_racing = :continue'
                       .' WHERE queueid = :queueid');
  $stmt->execute(array(':queueid' => $_POST['queueid'],
                       ':top' => $top,
                       ':bucketed' => $bucketed,
                       ':ntimes' => $n_times_per_lane,
                       ':sceneid' => $sceneid_at_finish,
                       ':continue' => $continue_racing));
  json_success();
} else {
  json_not_authorized();
}

?>