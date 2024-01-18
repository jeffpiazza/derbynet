<?php

require_once('inc/write-config-files.inc');

// Returns the part of URL path that follows the PHP file.  E.g. for
//
// photo.php/head/file/80x80/mygreatphoto.jpg/20150204133502
//
// returns "/head/file/80x80/mygreatphoto.jpg/20150204133502"
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

// The Images directory contains selectable subfolders for different image sets.
// "Generic" is one such image set.
function image_base_dir() {
  return  dirname(__FILE__, 2).DIRECTORY_SEPARATOR."Images";
}

// The image $stemname (e.g., "emblem") omits the file extension
function image_file_path($stemname) {
  global $homedir;
  // Look in folder for this specific event, first
  if (isset($homedir)) {
    $g = glob($homedir.DIRECTORY_SEPARATOR.'imagery'.DIRECTORY_SEPARATOR.$stemname.".*");
    if ($g !== false && count($g) > 0) {
      return $g[0];
    }
  }
  // Then look in top-level folder, shared across the installation
  $g = glob(data_folder_file_path('imagery').DIRECTORY_SEPARATOR.$stemname.".*");
  if ($g !== false && count($g) > 0) {
    return $g[0];
  }
  // Finally, look in selected image set
  $g = glob(image_base_dir().DIRECTORY_SEPARATOR
       .read_raceinfo("images-dir", 'Generic').DIRECTORY_SEPARATOR
            .$stemname.".*");
  if ($g !== false && count($g) > 0) {
    return $g[0];
  }
  // Shouldn't get here
  return false;
}

// $slide_glob may or may not include '*' wildcards
function slide_file_glob($slide_glob) {
  $g = glob($slide_glob);
  if ($g !== false && count($g) > 0) {
    return $g[0];
  }
  return false;
}

function slide_file_path($slide_glob) {
  global $homedir;
  // $homedir points to the data for the particular instance
  if (isset($homedir)) {
    $cand = slide_file_glob($homedir.DIRECTORY_SEPARATOR.'slides'.DIRECTORY_SEPARATOR.$slide_glob);
    if ($cand) return $cand;
  }

  // If nothing there, then look to in the "global" data folder, applying to all instances
  $cand = slide_file_glob(data_folder_file_path('slides').DIRECTORY_SEPARATOR.$slide_glob);
  if ($cand) return $cand;

  // Finally, look in the 'slides' subfolder of the currently-selected image set
  $selected_image_set_dir = image_base_dir().DIRECTORY_SEPARATOR
                          .read_raceinfo("images-dir", 'Generic');
  return slide_file_glob($selected_image_set_dir.DIRECTORY_SEPARATOR
                         .'slides'.DIRECTORY_SEPARATOR.$slide_glob);
}

// Search for files in the $dir directory that alphabetically follow $current_slide
function enumerate_non_title_slides($dir, $current_slide) {
  $results = array();
  if (is_dir($dir) && is_readable($dir)) {
    $files = scandir($dir);  // Sorted by default
    foreach ($files as $file) {
      if ($file > $current_slide && !str_starts_with($file, '.') &&
          !str_starts_with($file, 'title.')) {
        $results[] = $file;
      }
    }
  }
  return $results;
}

// Returns full file names (not stems) that follow (alphabetically) the passed-in file name, $f.
// Returns are returned without paths, after looking through all available slide directories.
function next_slides($f) {
  global $homedir;
  $result = array_merge(
    // Instance-specific directory
    (!isset($homedir) ? array() :
     enumerate_non_title_slides($homedir.DIRECTORY_SEPARATOR.'slides', $f)),
    // Global data directory
    enumerate_non_title_slides(data_folder_file_path('slides'), $f)
    // Image sets provide ONLY a title slide, so no need to look there.
  );
  sort($result);
  return $result;
}

?>
