<?php @session_start(); ?>
<?php

require_once('inc/data.inc');
session_write_close();
require_once('inc/timer-logging.inc');

if ($_SERVER['REQUEST_METHOD'] == 'POST') {
  header('Content-Type: text/xml; charset=utf-8');
  echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  $post_data = fopen('php://input', 'r');
  $log_file = read_raceinfo('timer-log');
  if (!$log_file) {
    $log_file = set_timer_log_file();
  }
  $log = fopen($log_file, 'a+');
  if ($log !== false) {
    flock($log, LOCK_EX);
    $nbytes = stream_copy_to_stream($post_data, $log);
    flock($log, LOCK_UN);
    fclose($log);
    echo "<success>$nbytes bytes</success>\n";
  } else {
    echo "<failure>Can't open ".read_raceinfo('timer-log')."\n";
    echo json_encode(error_get_last(), JSON_HEX_TAG | JSON_HEX_AMP | JSON_PRETTY_PRINT);
    echo "</failure>\n";
    // Pick a new timer log file
    set_timer_log_file();
  }
} else {
    echo '<!DOCTYPE html><html><head><title>Not a Page</title></head><body><h1>This is not a page.</h1></body></html>';
}

?>
