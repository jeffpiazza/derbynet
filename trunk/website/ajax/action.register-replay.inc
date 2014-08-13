<?php
// $_POST['port']
//
// TODO: Permissions?  Presently accepting registration action without authentication.

require_once('inc/replay.inc');

start_response();

$ip_addr = $_SERVER['REMOTE_ADDR'];
$port = $_POST['port'];

if (strpos($ip_addr, ":") !== false) {
  if ($ip_addr == "::1") {
    $ip_addr = "localhost";
  } else {
    // fsockopen needs these brackets for IPv6 addresses.  It may not work even then.
    $ip_addr = "[".$ip_addr."]";
  }
}

$replay = get_replay();

if ($replay['host_and_port'] == $ip_addr.'$'.$port  && $replay['state'] == REPLAY_CONNECTED) {
  echo "<ignored/>\n";
  write_raceinfo('replay_last_contact', time());
  echo "<success/>\n";
} else {
  if (send_replay_message_to_address($ip_addr, $port, "HELLO")) {
    write_raceinfo('replay_host_and_port', $ip_addr.'$'.$port);
    echo '<registered port="'.$port.'">'.$ip_addr."</registered>\n";
    echo "<success/>\n";
  } else {
    echo "<failure code='trouble'>".read_raceinfo('replay_trouble')."</failure>\n";
  }
}

end_response();
?>
