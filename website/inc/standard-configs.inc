<?php

// Searches for "standard configurations": directories with an identifiable
// database, racers directory, and cars directory.

function has_database_file($path) {
  $db = $path . DIRECTORY_SEPARATOR . 'derbynet.sqlite3';
  if (file_exists($db) && is_readable($db)) {
    return $db;
  }
  
  $db = $path . DIRECTORY_SEPARATOR . basename($path) . '.sqlite3';
  // This insists that the database name exactly match the immediate directory
  // name.
  if (file_exists($db) && is_readable($db)) {
    return $db;
  }
  return false;
}

// Returns true if $path has a subdirectory whose name matches one of the
// strings in $subdirs.
function has_subdirectory($path, $subdirs) {
  foreach ($subdirs as $s) {
    $subdir = $path . DIRECTORY_SEPARATOR . $s;
    if (is_dir($subdir) && is_readable($subdir)) {
      return $s;
    }
  }
  return false;
}

function is_standard_config($basepath, $relpath) {
  $path = /*realpath*/($basepath . DIRECTORY_SEPARATOR . $relpath);
  if (is_dir($path)) {
    if (($db = has_database_file($path)) === false) {
      return false;
    }
    if (($racers = has_subdirectory($path, array('Racers', 'racers'))) === false) {
      return false;
    }
    if (($cars = has_subdirectory($path, array('Cars', 'cars'))) === false) {
      return false;
    }
    return array('relpath' => $relpath,
                 'database' => $db,
                 'racers' => $racers,
                 'cars' => $cars);
  }
  return false;
}

function recur_for_standard_configs($basepath, $relpath, &$results) {
  if (($s = is_standard_config($basepath, $relpath)) !== false) {
    $results[] = $s;
  } else {
    $path = /*realpath*/($basepath . DIRECTORY_SEPARATOR . $relpath);
    if (is_dir($path) && is_readable($path)) {
      if ($dh = opendir($path)) {
        while (($file = readdir($dh)) !== false) {
          if ($file != "." && $file != "..") {
            recur_for_standard_configs($basepath, $relpath . DIRECTORY_SEPARATOR . $file, $results);
          }
        }
        closedir($dh);
      }
    }
  }
}
    
function list_standard_configs($basepath) {
  $basepath = realpath($basepath);
  $results = array();
  if (is_dir($basepath) && is_readable($basepath)) {
    if ($dh = opendir($basepath)) {
      while (($file = readdir($dh)) !== false) {
        if ($file != "." && $file != "..") {
          recur_for_standard_configs($basepath, $file, $results);
        }
      }
      closedir($dh);
    }
  }
  return $results;
}

?>