<?php
// TODO: Permissions?  Presently accepting registration action without authentication.

require_once('inc/replay.inc');

start_response();

function drain_queue() {
  global $db;

  $db->beginTransaction();
  $queue = read_raceinfo('replay_queue');
  write_raceinfo('replay_queue', '');
  $db->commit();
  $db->setAttribute(PDO::ATTR_AUTOCOMMIT, true);

  foreach (explode("\n", $queue) as $msg) {
    if (strlen($msg) > 0) {
      echo "<replay-message>".htmlspecialchars($msg, ENT_QUOTES, 'UTF-8')."</replay-message>\n";
    }
  }
}

$replay = get_replay();

if ($replay['state'] == REPLAY_CONNECTED) {
  echo "<ignored/>\n";
} else {
  write_raceinfo('replay_state', REPLAY_CONNECTED);
  echo "<replay-message>HELLO</replay-message>\n";  // TODO
}

write_raceinfo('replay_last_contact', time());
echo "<success/>\n";
drain_queue();

end_response();
?>
