<?php
/*
{original:, path:, base-paths: [{path, basename}, ...], files: [{file:, readable:, writable:, isdir:}, ...]}
*/

if (!isset($_GET['path'])) {
  json_failure('nopath', "No PATH query argument provided");
} else {
  $dirpath = $_GET['path'];
  json_out('original', $dirpath);

  while (is_link($dirpath)) {
    $dirpath = readlink($dirpath);
  }

  if (is_file($dirpath)) {
    $basename = pathinfo($dirpath, PATHINFO_BASENAME);
    $dirpath = pathinfo($dirpath, PATHINFO_DIRNAME);
    json_out('chosen', $basename);
  }
  
  while (true) {
    if ($dirpath === "") {
      $dirpath = DIRECTORY_SEPARATOR;
    }
    $files = @scandir($dirpath);
    if ($files !== false) {
      break;
    }
    json_failure('unable', "Unable to list directory $dirpath");
    $info = pathinfo($dirpath, PATHINFO_DIRNAME);
    if ($dirpath == $info) {
      // Nothing left
      break;
    } else {
      $dirpath = $info;
    }
  }

  if (substr($dirpath, -1) !== DIRECTORY_SEPARATOR) {
    $dirpath .= DIRECTORY_SEPARATOR;
  }

  $realpath = realpath($dirpath);
  if (substr($realpath, -1) !== DIRECTORY_SEPARATOR) {
    $realpath .= DIRECTORY_SEPARATOR;
  }
  json_out('path', $realpath);

  $base_paths = array();
  $superpath = $realpath;
  while ($superpath !== "") {
    $basename = pathinfo($superpath, PATHINFO_BASENAME);
    $base_paths[] = array('path' => $superpath,
                          'basename' => $basename);
    $info = pathinfo($superpath, PATHINFO_DIRNAME);
    if ($superpath == $info) {
      break;
    } else {
      $superpath = $info;
    }
  }
  json_out('base-paths', $base_paths);

  $file_list = array();
  if ($files === false) {
  } else {
    foreach ($files as $file) {
      // if ($file != "." && $file != "..") {
      if (substr($file, 0, 1) != ".") {
        $path = $dirpath . $file;
        $file_list[] = array('file' => $file,
                             'readable' => is_readable($path),
                             'writable' => is_writable($path),
                             'isdir' => is_dir($path));
      }
    }
  }
  json_out('files', $file_list);
}
?>
