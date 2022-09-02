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

// $lhs and $rhs are expected to be strings of the form {year}/{group}.  Order
// by decreasing year, increasing group.  Otherwise just compare as strings.
function compare_by_relpath($lhs_config, $rhs_config) {
  $lhs_string = $lhs_config['relpath'];
  $rhs_string = $rhs_config['relpath'];
  $lhs = explode(DIRECTORY_SEPARATOR, $lhs_string);
  $rhs = explode(DIRECTORY_SEPARATOR, $rhs_string);
  $n = count($lhs);
  if ($n >= 2 && $n == count($rhs)) {
    if ($lhs[$n - 2] == $rhs[$n - 2]) { // Same year, compare by group name
      return strcmp($lhs[$n - 1], $rhs[$n - 1]);
    } else {
      return -strcmp($lhs[$n - 2], $rhs[$n - 2]);
    }
  } else {
    return strcmp($lhs_string, $rhs_string);
  }
}

// Tests whether basepath + relpath represents a standard configuration
// directory containing a database and folders for racer and car photos.
// Returns {relpath, database, racers, cars} if a standard configuration; false
// if not.
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
                 'homedir' => $path,
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
  usort($results, 'compare_by_relpath');
  return $results;
}

?>
