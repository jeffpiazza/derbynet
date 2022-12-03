<?php

// The presence of a settings.lock file in the config directory prevents changes to
// the database and the photo directories.  This is specifically intended to support hosting
// for multiple groups on a single shared hosting account.

function locked_settings() {
  $configdir = isset($_SERVER['DERBYNET_CONFIG_DIR']) ? $_SERVER['DERBYNET_CONFIG_DIR'] : 'local';
  return file_exists($configdir.DIRECTORY_SEPARATOR.'settings.lock');
}

function preferred_urls_from_file() {
  $configdir = isset($_SERVER['DERBYNET_CONFIG_DIR']) ? $_SERVER['DERBYNET_CONFIG_DIR'] : 'local';
  try {
    // If present, the config-url should contain the whole url, e.g.,
    //  http://my-host.com/with/a/long/path
    $config_url_file = $configdir.DIRECTORY_SEPARATOR."config-url";
    if (!file_exists($config_url_file)) {
      return false;
    }
    $v = file_get_contents($config_url_file);
    if ($v !== false) {
      return array(trim($v));
    }
  } catch (Exception $p) {
  }
  return false;
}

function preferred_urls() {
  $urls = preferred_urls_from_file();
  if ($urls === false) {
    // gethostname() may be something like "instance-1", possibly with a non-routable IP.
    $addrs = gethostbynamel(gethostname());
    $urls = array();

    // IIS apparently doesn't set REQUEST_URI.
    if (isset($_SERVER['REQUEST_URI'])) {
      $uri = dirname($_SERVER['REQUEST_URI']);
    } else {
      $uri = '/...';
    }

    for ($i = 0; $i < count($addrs); ++$i) {
      $urls[] = $_SERVER['REQUEST_SCHEME']."://".$addrs[$i].$uri;
    }
  }
  if (count($urls) == 0) $urls = array("");
  return $urls;
}

?>
