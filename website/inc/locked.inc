<?php

// The presence of a settings.lock file in the config directory prevents changes to
// the database and the photo directories.  This is specifically intended to support hosting
// for multiple groups on a single shared hosting account.

function locked_settings() {
  $configdir = isset($_SERVER['CONFIG_DIR']) ? $_SERVER['CONFIG_DIR'] : 'local';
  return file_exists($configdir.DIRECTORY_SEPARATOR.'settings.lock');
}

function preferred_urls() {
  $configdir = isset($_SERVER['CONFIG_DIR']) ? $_SERVER['CONFIG_DIR'] : 'local';
  try {
    // If present, the config-url should contain the whole url, e.g.,
    //  http://my-host.com/with/a/long/path
    $v = file_get_contents($configdir.DIRECTORY_SEPARATOR."config-url");
    if ($v !== false) {
      return array(trim($v));
    }
  } catch (Exception $p) {
  }
  return false;
}

?>