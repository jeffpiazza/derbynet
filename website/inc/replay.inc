<?php

// TODO: Allow separate enable/disable of replay, so the address/port
// can be preserved even if disabled.  Meanwhile, just put a blank in
// replay_host_and_port to disable replay.
//
// TODO: Controllable replay command

// replay host and port has format 'host:port'.  

require_once('inc/data.inc');


function write_replay_host_and_port($host_and_port) {
  write_raceinfo('replay_host_and_port', $host_and_port);
}

function read_replay_host_and_port() {
  return read_raceinfo('replay_host_and_port', '');
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

    fclose($fp);
  }
}

?>