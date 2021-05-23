<?php
// $_POST['status']
// $_POST['finished-replay'] (0/1)
// TODO: Permissions?  Presently accepting registration action without authentication.

require_once('inc/replay.inc');

function not_empty_string($s) {
  return $s !== "";
}

function drain_queue() {
  global $db;

  $db->beginTransaction();
  $queue = read_raceinfo('replay_queue');
  write_raceinfo('replay_queue', '');
  $db->commit();
  try {
    $db->setAttribute(PDO::ATTR_AUTOCOMMIT, true);
  } catch (PDOException $p) {
  }

  return array_filter(explode("\n", $queue), 'not_empty_string');
}

$replay = get_replay();

if ($replay['state'] != REPLAY_CONNECTED) {
  write_raceinfo('replay_state', REPLAY_CONNECTED);
}

write_raceinfo('replay_reported', $_POST['status']);
write_raceinfo('replay_last_contact', time());

json_success();

$queue = drain_queue();
if ($replay['state'] != REPLAY_CONNECTED) {
  prepend($queue, 'HELLO');
}
json_out('replay', $queue);
?>
