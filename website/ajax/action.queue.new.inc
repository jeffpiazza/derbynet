<?php

// $_POST['roundid'] or $_POST['n_times_per_lane']
// $_POST['sceneid_at_finish'] or $_POST['continue_racing'] or nothing

start_response();

if (have_permission(SET_UP_PERMISSION)) {
  $ok = true;
  if (!isset($_POST['roundid'])) {
    echo "<failure code='no_roundid'/>\n";
    $ok = false;
  } else {
    $roundid = $_POST['roundid'];
  }
  if (isset($_POST['n_times_per_lane'])) {
    $n_times_per_lane = $_POST['n_times_per_lane'];
  } else {
    $n_times_per_lane = -1;
  }

  $continue_racing = 0;
  if ($ok && isset($_POST['sceneid_at_finish'])) {
    $sceneid_at_finish = $_POST['sceneid_at_finish'];
    $continue_racing = 0;
  } else if ($ok && isset($_POST['continue_racing'])) {
    $sceneid_at_finish = -1;
    $continue_racing = $_POST['continue_racing'] ? 1 : 0;
  } else {
    $sceneid_at_finish = -1;
    $continue_racing = 0;
  }

  if ($ok) {
    $new_seq = 1 + read_single_value('SELECT MAX(seq) FROM RacingQueue', array(), 0);
    $stmt = $db->prepare('INSERT INTO RacingQueue(seq, roundid, n_times_per_lane,'
                         .'                       sceneid_at_finish, continue_racing)'
                         .' VALUES(:newseq, :roundid, :n_times_per_lane,'
                         .'        :sceneid_at_finish, :continue_racing)');
    $stmt->execute(array(':newseq' => $new_seq,
                         ':roundid' => $roundid,
                         ':n_times_per_lane' => $n_times_per_lane,
                         ':sceneid_at_finish' => $sceneid_at_finish,
                         ':continue_racing' => $continue_racing));
    echo "<success/>\n";
  }
                       
} else {
  not_authorized_body();
}

end_response();

?>