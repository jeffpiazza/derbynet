<?php

require_once('inc/write-config-files.inc');

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
  // Look in folder for this specific event, first
  if (isset($homedir)) {
    $g = glob($homedir.DIRECTORY_SEPARATOR.'imagery'.DIRECTORY_SEPARATOR.$filename.".*");
    if ($g !== false && count($g) > 0) {
      return $g[0];
    }
  }
  // Then look in top-level folder, shared across the installation
  $g = glob(data_folder_file_path('imagery').DIRECTORY_SEPARATOR.$filename.".*");
  if ($g !== false && count($g) > 0) {
    return $g[0];
  }
  // Finally, look in selected image set
  $g = glob(image_base_dir().DIRECTORY_SEPARATOR
       .read_raceinfo("images-dir", 'Generic').DIRECTORY_SEPARATOR
            .$filename.".*");
  if ($g !== false && count($g) > 0) {
    return $g[0];
  }
  // Shouldn't get here
  return $filename;
}

function slide_file_path($filename) {
  global $homedir;
  if (isset($homedir)) {
    $g = glob($homedir.DIRECTORY_SEPARATOR.'slides'.DIRECTORY_SEPARATOR.$filename.".*");
    if ($g !== false && count($g) > 0) {
      return $g[0];
    }
  }
  $g = glob(data_folder_file_path('slides').DIRECTORY_SEPARATOR.$filename.".*");
  if ($g !== false && count($g) > 0) {
    return $g[0];
  }

  // Slides that aren't customized are found in a subfolder of the image set
  $g = glob(image_base_dir().DIRECTORY_SEPARATOR
       .read_raceinfo("images-dir", 'Generic').DIRECTORY_SEPARATOR
       .'slides'.DIRECTORY_SEPARATOR.$filename.".*");
  if ($g !== false && count($g) > 0) {
    return $g[0];
  }
  // Shouldn't get here
  return $filename;
}

function enumerate_non_title_slides($dir, $current_slide) {
  $results = array();
  if (is_dir($dir) && is_readable($dir)) {
    if ($dh = opendir($dir)) {
      while (($file = readdir($dh)) !== false) {
        if ($file > $current_slide && !str_starts_with($file, '.') &&
            // TODO Check $file = 'title.'.pathinfo(...$file)['extension'] ?
            !str_starts_with($file, 'title.')) {
          $results[] = $file;
        }
      }
    }
    closedir($dh);
  }
  return $results;
}

function next_slides($f) {
  global $homedir;
  // TODO realpath
  $results = array();

  if (isset($homedir)) {
    $results = array_merge($results, enumerate_non_title_slides($dir, $f));
  }
  $results = array_merge($results, enumerate_non_title_slides(data_folder_file_path('slides'), $f));

  return $results;
}

?>
