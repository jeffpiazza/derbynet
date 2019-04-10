<?php
session_start(); 

require_once('inc/photo-config.inc');

// URL for a file in one of the repositories is:
//  photo.php/<repository>/file/<render>/<file-basename>/<cachebreaker>, e.g.
//  photo.php/head        /file/80x80   /mygreatphoto.jpg/20150204133502
//
// URL for an arbitrary file named by a racer's database entry is:
//  photo.php/<repository>/racer/<racerid>/<cachebreaker>
// A racer-based URL requires another database fetch to get the file path, and
// then returns exactly that file.
//
// URL for a file named by a RaceInfo key:
//  photo.php/info/<key>/<cachebreaker>/<default img path>

// parse_photo_url returns:
//    { 'repository', 'url_type' }, and, depending, possibly other keys.
function parse_photo_url($url_path_info) {
  $exploded = explode('/', $url_path_info);
  if ($exploded[2] == 'file') {
    return array('repository' => $exploded[1],
                 'url_type' => $exploded[2], // 'file'
                 'render' => $exploded[3],
                 'basename' => urldecode($exploded[4]));
  } else if ($exploded[2] == 'racer') {
    return array('repository' => $exploded[1],
                 'url_type' => $exploded[2], // 'racer'
                 'racerid' => $exploded[3]);
  } else if ($exploded[1] == 'info') {
    return array('repository' => 'info',
                 'url_type' => 'info',
                 'key' => $exploded[2],
                 'default' => implode('/', array_slice($exploded, 4)));
  } else {
    return false;
  }
}

if (isset($_SERVER['PATH_INFO'])) {
  $path_info = $_SERVER['PATH_INFO'];
} else if (isset($_SERVER['REQUEST_URI']) && isset($_SERVER['SCRIPT_NAME']) &&
           substr($_SERVER['REQUEST_URI'], 0, strlen($_SERVER['SCRIPT_NAME'])) == $_SERVER['SCRIPT_NAME']) {
  $path_info = substr($_SERVER['REQUEST_URI'], strlen($_SERVER['SCRIPT_NAME']));
} else if (isset($_SERVER['ORIG_PATH_INFO'])) {
  // Rewrite rules in Apache 2.2 may leave ORIG_PATH_INFO instead of PATH_INFO
  $path_info = 'photo.php'.$_SERVER['ORIG_PATH_INFO'];
} else {
  // Debugging only:
  var_export($_SERVER);
  exit(0);
}

$parsed = parse_photo_url($path_info);
if (!$parsed) {  // Malformed URL
  http_response_code(404);
  exit(1);
}

if ($parsed['url_type'] == 'info') {
  $file_path = read_raceinfo($parsed['key'], $parsed['default']);
} else {
  $repo = photo_repository($parsed['repository']);
  if (!$repo) {  // No such repository
    http_response_code(404);
    exit(1);
  }

  if ($parsed['url_type'] == 'racer') {
    $file_path = read_single_value('SELECT '.$repo->column_name()
                                   .' FROM RegistrationInfo'
                                   .' WHERE racerid = :racerid',
                                   array(':racerid' => $parsed['racerid']));
  } else {
    $render = $repo->lookup($parsed['render']);
    if (!$render) {  // No such render
      http_response_code(404);
      exit(1);
    }
    $file_path = $render->find_or_make_image_file($parsed['basename']);
  }
}

if (!$file_path) {  // No such racer/no such photo
  http_response_code(404);
  exit(1);
}

header('Pragma: public');
header('Cache-Control: max-age=86400, public');
header('Expires: '.gmdate('D, d M Y H:i:s', time() + 86400).' GMT');

header('Content-type: '.pseudo_mime_content_type($file_path));

readfile($file_path);
?>