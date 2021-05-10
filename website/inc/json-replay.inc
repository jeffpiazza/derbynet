<?php

require_once('inc/replay.inc');

function json_replay() {
  $replay = get_replay();
  if ($replay['state'] == REPLAY_NOT_CONNECTED) {
    $msg = 'NOT CONNECTED';
    $icon = 'img/status/not_connected.png';
  } else if ($replay['state'] == REPLAY_CONNECTED) {
    $msg = $replay['reported_status'].' ('.(time() - $replay['last_contact']).' sec.)';
    $icon = 'img/status/ok.png';
  } else {
    $msg = 'TROUBLE: '.$replay['trouble'];
    $icon = 'img/status/trouble.png';
  }
  return array(
    'last_contact' => $replay['last_contact'],
    'state' => $replay['state'],
    'icon' => $icon,
    'connected' => $replay['state'] != REPLAY_NOT_CONNECTED,
    'message' => $msg);
}

?>