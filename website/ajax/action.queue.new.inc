<?php

// $_POST['classid']
// $_POST['round']

// Parameters for roster.new, if needed
// $_POST['top']
// $_POST['bucketed'] (0 or 1)

// Parameters for schedule.generate, if needed
// $_POST['n_times_per_lane']
// $_POST['sceneid_at_finish'] or $_POST['continue_racing'] or nothing

start_response();

if (have_permission(SET_UP_PERMISSION)) {
  $ok = true;
  if (!isset($_POST['classid'])) {
    echo "<failure code='no_classid'>No classid</failure>\n";
    $ok = false;
  } else {
    $classid = $_POST['classid'];
  }
  if (!isset($_POST['round'])) {
    echo "<failure code='no_round'>No round</failure>\n";
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
    echo "<success/>\n";
    echo "<queue-entry>".json_encode(
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
                      PDO::FETCH_ASSOC),
      JSON_HEX_TAG | JSON_HEX_AMP | JSON_PRETTY_PRINT)."</queue-entry>\n";
  }
} else {
  not_authorized_body();
}

end_response();

?>