<?php
// Installer may leave a file here to set $default_file_path variable

function default_file_path() {
  $configdir = isset($_SERVER['CONFIG_DIR']) ? $_SERVER['CONFIG_DIR'] : 'local';
  $local_default_file_path_inc = $configdir.DIRECTORY_SEPARATOR.'default-file-path.inc';
  if (file_exists($local_default_file_path_inc)) {
    try {
      @include($local_default_file_path_inc);
    } catch (Exception $e) {
      unset($default_file_path);
    }
    return isset($default_file_path) ? $default_file_path : "";
  }
}
?>