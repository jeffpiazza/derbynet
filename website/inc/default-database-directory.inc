<?php
// Installer may leave a file to suggest where to put the database and/or photo directories.

function default_database_directory() {
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