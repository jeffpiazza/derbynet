<?php

// The presence of a settings.lock file in the config directory prevents changes to
// the database and the photo directories.  This is specifically intended to support hosting
// for multiple groups on a single shared hosting account.

function locked_settings() {
  $configdir = isset($_SERVER['CONFIG_DIR']) ? $_SERVER['CONFIG_DIR'] : 'local';
  return file_exists($configdir.DIRECTORY_SEPARATOR.'settings.lock');
}

?>