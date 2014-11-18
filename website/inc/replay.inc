<?php

// replay_last_contact

// replay_state:
define('REPLAY_NOT_CONNECTED', 1);
define('REPLAY_CONNECTED', 2);  // See replay_last_contact.
define('REPLAY_TROUBLE', 3);   // See replay_trouble value

// Registration request is ignored if REPLAY_CONNECTED and the host
// and port match.  Otherwise, a registration request causes a HELLO
// message to be sent, and the state becomes either CONNECTED or
// TROUBLE depending on the outcome.

// replay_last_contact gets updated with succesful message sent.
// replay_trouble gets set to the error message for an unsuccessful
// message send.

// REPLAY_CONNECTED or REPLAY_TROUBLE become REPLAY_NOT_CONNECTED if
// REPLAY_TIMEOUT_LIMIT passes since replay_last_contact.
//
// REPLAY_TROUBLE causes message sends to be ignored (except
// re-registration attempts).  Re-registration attempt also sets
// replay_last_contact.


require_once('inc/data.inc');

define('REPLAY_TIMEOUT_LIMIT', 10);

// TODO
function get_replay() {
  $state = read_raceinfo('replay_state', REPLAY_NOT_CONNECTED);
  $reported_status = read_raceinfo('replay_reported', 0);
  $last_contact = read_raceinfo('replay_last_contact', 0);
  if ($last_contact + REPLAY_TIMEOUT_LIMIT < time()) {
    $state = REPLAY_NOT_CONNECTED;
  }
  $trouble = read_raceinfo('replay_trouble');

  $status_array = array(
         0 => 'Ready',
         1 => 'Recording',
         2 => 'Playing',
         -1 => 'Connecting...',
         -2 => 'No video source chosen',
         -3 => 'No audio source chosen',
         -4 => 'Recording error');

  return array('state' => $state,
               'reported_status' => $status_array[$reported_status],
               'last_contact' => $last_contact,
               'trouble' => $trouble);
}


function send_replay_message($cmd) {
  global $db;
  $replay = get_replay();
  if ($replay['state'] == REPLAY_CONNECTED) {
    $db->beginTransaction();
    write_raceinfo('replay_queue', read_raceinfo('replay_queue', '').$cmd."\n");
    $db->commit();
    $db->setAttribute(PDO::ATTR_AUTOCOMMIT);
    return true;
  } else {
    return false;
  }
}

function send_replay_TEST() {
  // test <skipback_seconds> <repeat> <rate>
  send_replay_message("TEST 4 2 0.5");
}

// "START video_name_root  -- start recording video\r\n"
function send_replay_START(&$current) {
  send_replay_message("START ".@$current['class']
                      .'_Round'.@$current['round']
                      .'_Heat'.sprintf("%02d", @$current['heat']));
}

// "REPLAY skipback showings rate  -- stop recording if recording; playback\r\n"
function send_replay_REPLAY() {
  // TODO: Settings to control what gets sent
  send_replay_message("REPLAY 4 2 0.5");
}

// "CANCEL  -- cancel recording\r\n"
function send_replay_CANCEL() {
  send_replay_message("CANCEL");
}

?>
