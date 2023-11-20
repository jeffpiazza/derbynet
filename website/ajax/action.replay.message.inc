<?php
// $_POST['status']
// $_POST['finished-replay'] (0/1)
// TODO: Permissions?  Presently accepting registration action without authentication.

require_once('inc/replay.inc');

function not_empty_string($s) {
  return $s !== "";
}

$replay = get_replay();

if ($replay['state'] != REPLAY_CONNECTED) {
  write_raceinfo('replay_state', REPLAY_CONNECTED);
}

write_raceinfo('replay_reported', $_POST['status']);
write_raceinfo('replay_last_contact', time());

$queue = drain_queue();
if ($replay['state'] != REPLAY_CONNECTED) {
  array_unshift($queue, 'HELLO');
}
json_out('replay', $queue);

json_success();
?>
