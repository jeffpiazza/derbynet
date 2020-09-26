<?php

function path_info() {
  if (isset($_SERVER['PATH_INFO'])) {
    return $_SERVER['PATH_INFO'];
  } else if (isset($_SERVER['REQUEST_URI']) && isset($_SERVER['SCRIPT_NAME']) &&
             substr($_SERVER['REQUEST_URI'], 0, strlen($_SERVER['SCRIPT_NAME'])) == $_SERVER['SCRIPT_NAME']) {
    return substr($_SERVER['REQUEST_URI'], strlen($_SERVER['SCRIPT_NAME']));
  } else if (isset($_SERVER['ORIG_PATH_INFO'])) {
    // Rewrite rules in Apache 2.2 may leave ORIG_PATH_INFO instead of PATH_INFO
    return 'photo.php'.$_SERVER['ORIG_PATH_INFO'];
  } else {
    // Debugging only:
    var_export($_SERVER);
    exit(0);
  }
}
?>