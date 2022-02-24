<?php

// $_POST['classid']
// $_POST['round']
//
// $_POST['sceneid_at_finish'] (optional)
// $_POST['continue_racing'] (optional)

// Parameters for roster.new, if needed
// $_POST['top']
// $_POST['bucketed'] (0 or 1)

// Parameters for schedule.generate, if needed
// $_POST['n_times_per_lane']


if (have_permission(SET_UP_PERMISSION)) {
  $ok = true;
  if (!isset($_POST['classid'])) {
    json_failure('no_classid', "No classid");
    $ok = false;
  } else {
    $classid = $_POST['classid'];
  }
  if (!isset($_POST['round'])) {
    json_failure('no_round', "No round");
    $ok = false;
  } else {
    $round = $_POST['round'];
  }

  if (isset($_POST['top'])) {
    $top = $_POST['top'];
  } else {
    $top = 0;
  }
  if (isset($_POST['bucketed'])) {
    $bucketed = $_POST['bucketed'];
  } else {
    $bucketed = false;
  }

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

  if ($ok) {
    $new_seq = 1 + read_single_value('SELECT MAX(seq) FROM Playlist', array(), 0);
    $stmt = $db->prepare('INSERT INTO Playlist(seq, classid, round,'
                         .' bucket_limit, bucketed,'
                         .' n_times_per_lane, sceneid_at_finish, continue_racing)'
                         .' VALUES(:newseq, :classid, :round,'
                         .'        :top, :bucketed, :n_times_per_lane,'
                         .'        :sceneid_at_finish, :continue_racing)');
    $stmt->execute(array(':newseq' => $new_seq,
                         ':classid' => $classid,
                         ':round' => $round,
                         ':top' => $top,
                         ':bucketed' => $bucketed ? 1 : 0,
                         ':n_times_per_lane' => $n_times_per_lane,
                         ':sceneid_at_finish' => $sceneid_at_finish,
                         ':continue_racing' => $continue_racing));
    json_out('queue-entry',
      read_single_row('SELECT queueid, seq, Playlist.classid, Playlist.round,'
                      .' bucket_limit, bucketed,'
                      .' n_times_per_lane, sceneid_at_finish, continue_racing,'
                      .' Classes.classid, class, round,'
                      .(use_groups() ? "class || ', ' || " : "")
                      .'\'Round \' || round AS roundname'
                      .' FROM '.inner_join('Playlist', 'Classes',
                                           'Playlist.classid = Classes.classid')
                      .' WHERE seq = :seq',
                      array(':seq' => $new_seq),
                      PDO::FETCH_ASSOC));
    json_success();
  }
} else {
  json_not_authorized();
}

?>