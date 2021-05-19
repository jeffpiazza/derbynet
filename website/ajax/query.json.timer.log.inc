<?php

$seek = $_GET['seek'];

$timer_log_name = read_raceinfo('timer-log', '');
$log = $timer_log_name ? fopen($timer_log_name, 'r') : false;
if ($log === false) {
  json_failure('nolog', "No log file $timer_log_name");
  $size = 0;
} else {
  $stat = fstat($log);
  $size = $stat['size'];
  if ($size < $seek) {
    $size = 0;
  } else if ($size > $seek) {
    flock($log, LOCK_SH);
    if (fseek($log, $seek) != 0) {    }
    $data = fread($log, $size - $seek);
    flock($log, LOCK_UN);
    fclose($log);
    json_out('file-data', $data);
  }
}

json_out('file-size', $size);

?>