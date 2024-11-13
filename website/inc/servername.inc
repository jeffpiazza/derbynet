<?php

function server_name() {
  $server = '';

  // The $_SERVER super-global has lots of different ways to 
  // nginx frequently is configured to claim "_" as the server name.
  // http://nginx.org/en/docs/http/server_names.html

  if ((empty($server) || $server == '_') && isset($_SERVER['HTTP_HOST'])) {
    $server = $_SERVER['HTTP_HOST'];
  }
  if ((empty($server) || $server == '_') && isset($_SERVER['SERVER_NAME'])) {
    $server = $_SERVER['SERVER_NAME'];
  }

  // Reporting the IP address that the server thinks it has may or may not be
  // the address the client would need to use, but perhaps it's better than
  // nothing.
  if ((empty($server) || $server == '_') && isset($_SERVER['SERVER_ADDR'])) {
    $server = $_SERVER['SERVER_ADDR'];
  }
  if ((empty($server) || $server == '_') && isset($_SERVER['LOCAL_ADDR'])) {
    $server = $_SERVER['LOCAL_ADDR'];
  }

  // Not even sure this case is accessible
  if ((empty($server) || $server == '_')) {
    $addrs = gethostbynamel(gethostname());
    if (count($addrs) > 0) {
      $server = $addrs[0];
    }
  }

  return $server;
}

function request_path() {
  $path = '';
  if (isset($_SERVER['DOCUMENT_URI']) && !empty($_SERVER['DOCUMENT_URI'])) {
    // DOCUMENT_URI will leave off any query parameters
    $path = $_SERVER['DOCUMENT_URI'];
  } else if (isset($_SERVER['REQUEST_URI'])) {
    $path = $_SERVER['REQUEST_URI'];
  } else if (isset($_SERVER['PHP_SELF'])) {
    $path = $_SERVER['PHP_SELF'];
  } else {
    $path = $_SERVER['SCRIPT_NAME'];
  }
  return $path;
}
