<?php

function path_info() {
  if (isset($_SERVER['PATH_INFO'])) {
    return $_SERVER['PATH_INFO'];
  } else if (isset($_SERVER['REQUEST_URI']) && isset($_SERVER['SCRIPT_NAME']) &&
             strlen($_SERVER['REQUEST_URI']) > strlen($_SERVER['SCRIPT_NAME']) &&
             substr($_SERVER['REQUEST_URI'], 0, strlen($_SERVER['SCRIPT_NAME'])) == $_SERVER['SCRIPT_NAME']) {
    return substr($_SERVER['REQUEST_URI'], strlen($_SERVER['SCRIPT_NAME']));
  } else if (isset($_SERVER['PHP_SELF']) && isset($_SERVER['DOCUMENT_ROOT']) &&
             isset($_SERVER['SCRIPT_FILENAME']) &&
             substr($_SERVER['SCRIPT_FILENAME'], 0, strlen($_SERVER['DOCUMENT_ROOT'])) == $_SERVER['DOCUMENT_ROOT'] &&
             substr($_SERVER['PHP_SELF'], 0, strlen($_SERVER['SCRIPT_FILENAME']) - strlen($_SERVER['DOCUMENT_ROOT'])) ==
             substr($_SERVER['SCRIPT_FILENAME'],  strlen($_SERVER['DOCUMENT_ROOT']))) {
    // On Alpine, at least:
    // 'DOCUMENT_ROOT'   => '/var/www/html',
    // 'SCRIPT_FILENAME' => '/var/www/html/photo.php',
    // 'PHP_SELF'        =>              '/photo.php/head/file/original/Cub-3126.jpg/xyz'
    return substr($_SERVER['PHP_SELF'], strlen($_SERVER['SCRIPT_FILENAME']) - strlen($_SERVER['DOCUMENT_ROOT']));
  } else if (isset($_SERVER['ORIG_PATH_INFO'])) {
    // Rewrite rules in Apache 2.2 may leave ORIG_PATH_INFO instead of PATH_INFO
    return 'photo.php'.$_SERVER['ORIG_PATH_INFO'];
  } else {
    // Debugging only:
    var_export($_SERVER);
    exit(0);
  }
}


function image_base_dir() {
  return  dirname(__FILE__, 2).DIRECTORY_SEPARATOR."Images";
}

function image_file_path($filename) {
  global $homedir;
  if (isset($homedir)) {
    $dir = $homedir.DIRECTORY_SEPARATOR.'imagery'.DIRECTORY_SEPARATOR;
    $g = glob($dir.$filename.".*");
    if ($g !== false && count($g) > 0) {
      return $g[0];
    }
  }
  $dir = image_base_dir().DIRECTORY_SEPARATOR
       .read_raceinfo("images-dir", 'Generic').DIRECTORY_SEPARATOR;
  $g = glob($dir.$filename.".*");
  if ($g !== false && count($g) > 0) {
    return $g[0];
  }
  // Shouldn't get here
  return $dir.$filename;
}

function slide_file_path($filename) {
  global $homedir;
  if (isset($homedir)) {
    $dir = $homedir.DIRECTORY_SEPARATOR.'slides'.DIRECTORY_SEPARATOR;
    $g = glob($dir.$filename.".*");
    if ($g !== false && count($g) > 0) {
      return $g[0];
    }
  }
  $dir = image_base_dir().DIRECTORY_SEPARATOR
       .read_raceinfo("images-dir", 'Generic').DIRECTORY_SEPARATOR
       .'slides'.DIRECTORY_SEPARATOR;

  $g = glob($dir.$filename.".*");
  if ($g !== false && count($g) > 0) {
    return $g[0];
  }
  // Shouldn't get here
  return $dir.$filename;
}

?>
