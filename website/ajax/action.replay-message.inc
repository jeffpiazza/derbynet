<?php
// $_POST['status']
// $_POST['finished-replay'] (0/1)
// TODO: Permissions?  Presently accepting registration action without authentication.

require_once('inc/replay.inc');
require_once('inc/racing-state.inc');

start_response(); echo "\n";

function drain_queue() {
  global $db;

  $db->beginTransaction();
  $queue = read_raceinfo('replay_queue');
  write_raceinfo('replay_queue', '');
  $db->commit();
  try {
    $db->setAttribute(PDO::ATTR_AUTOCOMMIT, true);
  } catch (PDOException $p) {
    // TODO - This doesn't seem to work:var_dump($p)
  }

  foreach (explode("\n", $queue) as $msg) {
    if (strlen($msg) > 0) {
      echo "<replay-message>".htmlspecialchars($msg, ENT_QUOTES, 'UTF-8')."</replay-message>\n";
    }
  }
}

$replay = get_replay();

if (($replay['state'] != REPLAY_CONNECTED)||(!isset($_SESSION['replay-msg-hello']))) {
  $_SESSION['replay-msg-hello']=1;
  write_raceinfo('replay_state', REPLAY_CONNECTED);
  echo "<replay-message>HELLO</replay-message>\n";  // TODO
  if (get_racing_state()) { //send updated start if running when replay connects
    $current = get_running_round();
    echo "<replay-message>"."START ".@$current['class']
    .'_Round'.@$current['round']
    .'_Heat'.sprintf("%02d", @$current['heat'])."</replay-message>\n";
  }
}

write_raceinfo('replay_reported', $_POST['status']);

if ($_POST['status'] == REPLAY_REPORTED_PLAYING &&
    $replay['status'] != REPLAY_REPORTED_PLAYING) {
  // Record the timestamp for when Replay first claimed to be playing back
  write_raceinfo('replay_started_playing_at', time());
}
write_raceinfo('replay_last_contact', time());
write_raceinfo('replay_just_finished', 
               (isset($_POST['finished-replay']) && $_POST['finished-replay']) ? 1 : 0);
echo "<success/>\n";
drain_queue();

end_response();
?>
