<?php

function set_timer_log_file() {
    $logs_dir = read_raceinfo('logs-directory', '');
    if ($logs_dir !== '' && !is_dir($logs_dir)) {
      write_raceinfo('logs-directory', '');
      $logs_dir = '';
    }
    if ($logs_dir === '') {
      $logs_dir = sys_get_temp_dir();
    }
    if (!substr($logs_dir, -1) !== DIRECTORY_SEPARATOR) {
      $logs_dir = $logs_dir . DIRECTORY_SEPARATOR;
    }
    $new_log = $logs_dir . "timer-" . date('Ymd-Hi-s') . ".log";
    write_raceinfo("timer-log", $new_log);
    return $new_log;
}
?>
