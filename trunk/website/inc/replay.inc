<?php

// replay_host_and_port has format 'host:port'.  
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

define('REPLAY_TIMEOUT_LIMIT', 3600);  // 3600 seconds = 1 hour

function get_replay() {
  $host_and_port = read_raceinfo('replay_host_and_port', '');
  $state = read_raceinfo('replay_state', REPLAY_NOT_CONNECTED);
  $last_contact = read_raceinfo('replay_last_contact', 0);
  if ($host_and_port == '' || $last_contact + REPLAY_TIMEOUT_LIMIT < time()) {
    $state = REPLAY_NOT_CONNECTED;
  }
  $trouble = read_raceinfo('replay_trouble');

  return array('state' => $state,
               'host_and_port' => $host_and_port,
               'last_contact' => $last_contact,
               'trouble' => $trouble);
}



function send_replay_message_to_address($host, $port, $cmd) {
  // Limiting connection timeout to 3 seconds in a half-hearted
  // attempt to avoid problems of long blocking while trying to
  // connect to a stale replay address.
  $fp = @fsockopen($host, $port, $errno, $errstr, 3);
  if (!$fp) {
    write_raceinfo('replay_state', REPLAY_TROUBLE);
    write_raceinfo('replay_trouble', $errstr);
    write_raceinfo('replay_last_contact', time());
    return false;
  } else {
    stream_set_timeout($fp, 3);
    fwrite($fp, $cmd."\r\n");
    $text = "";
    while (($buffer = fgets($fp, 128)) !== false) {
      $text .= $buffer;
      $lines = explode("\r\n", $text);
      for ($i = 0; $i < count($lines) - 1; ++$i) {
        if ($lines[$i] == "OK") {
          fclose($fp);
          write_raceinfo('replay_state', REPLAY_CONNECTED);
          write_raceinfo('replay_last_contact', time());
          return true;
        }
      }
      $text = $lines[count($lines) - 1];
    }

    // Control reaching here means we timed out waiting for an "OK"
    // from the replay application.  Assume there's trouble.
    fclose($fp);

    write_raceinfo('replay_state', REPLAY_TROUBLE);
    write_raceinfo('replay_trouble', 'Timed out');
    write_raceinfo('replay_last_contact', time());

    return false;
  }
}



function send_replay_message($cmd) {
  $replay = get_replay();
  if ($replay['state'] == REPLAY_CONNECTED) {
    list($host, $port) = explode('$', $replay['host_and_port']);
    return send_replay_message_to_address($host, $port, $cmd);
  } else {
    return false;
  }
}

function send_replay_TEST() {
  // test <skipback_seconds> <repeat> <rate>
  send_replay_message("TEST 4 1 0.5");
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
