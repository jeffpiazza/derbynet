<?php

$seek = $_GET['seek'];

echo "<timer-log>\n";

$timer_log_name = read_raceinfo('timer-log', '');
$log = $timer_log_name ? fopen($timer_log_name, 'r') : false;
if ($log === false) {
  echo "<failure code='nolog'>No log file $timer_log_name</failure>";
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
    echo "<file-data>";
    echo htmlspecialchars($data, ENT_QUOTES, 'UTF-8');
    echo "</file-data>\n";
  }
}

echo "<file-size size=\"$size\"/>\n";

echo "</timer-log>";

?>