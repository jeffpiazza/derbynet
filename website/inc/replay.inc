<?php

// replay host and port has format 'host:port'.  

require_once('inc/data.inc');

define('REPLAY_TIMEOUT_LIMIT', 3600);  // 3600 seconds = 1 hour

function write_replay_host_and_port($host_and_port) {
  write_raceinfo('replay_host_and_port', $host_and_port);
  write_raceinfo('replay_last_contact', time());
}

function read_replay_host_and_port() {
  if (read_raceinfo('replay_last_contact', 0) + REPLAY_TIMEOUT_LIMIT < time()) {
    return '';
  }
  return read_raceinfo('replay_host_and_port', '');
}

function get_replay_state() {
  $last_contact = read_raceinfo('replay_last_contact');
  return array('host_and_port' => read_raceinfo('replay_host_and_port'),
               'last_contact' => $last_contact,
               'state' => ($last_contact < 0) ? 'TROUBLE' :
               ($last_contact + REPLAY_TIMEOUT_LIMIT < time() ? 'NOT CONNECTED' : 'CONNECTED'));
}

function send_replay_message($cmd) {
  $host_and_port = read_replay_host_and_port();
  if ($host_and_port == '')
    return;
  list($host, $port) = explode(':', $host_and_port);

  // Limiting connection timeout to 3 seconds in a half-hearted
  // attempt to avoid problems of long blocking while trying to
  // connect to a stale replay address.
  $fp = @fsockopen($host, $port, $errno, $errstr, 3);
  if (!$fp) {
    // TODO
    // echo "Failed: $errstr ($errno)<br />\n";
    return;
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
          return;
        }
      }
      $text = $lines[count($lines) - 1];
    }

    // Control reaching here means we timed out waiting for an "OK"
    // from the replay application.  Assume there's trouble.
    fwrite($fp, "Goodbye.\r\n");  // TODO
    write_raceinfo('replay_last_contact', -time());

    fclose($fp);
  }
}

function send_replay_TEST() {
  // test <skipback_seconds> <repeat> <rate>
  send_replay_message("TEST 4 1 0.5");
}

// "START video_name_root  -- start recording video\r\n"
function send_replay_START(&$current) {
  send_replay_message("START ".@$current['class'].'-Round'.@$current['round'].'-Heat'.sprintf("%02d", @$current['heat']));
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