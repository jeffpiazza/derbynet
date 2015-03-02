<?php
session_start(); 

require_once('inc/photo-config.inc');

// URL for a file in one of the repositories is:
//  photo.php/<repository>/file/<render>/<file-basename>/<cachebreaker>, e.g.
//  photo.php/head        /file/thumb   /mygreatphoto.jpg/20150204133502
//
// URL for an arbitrary file named by a racer's database entry is:
//  photo.php/<repository>/racer/<racerid>/<cachebreaker>
// A racer-based URL requires another database fetch to get the file path, and
// then returns exactly that file.

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
  } else {
    return false;
  }
}

$parsed = parse_photo_url($_SERVER['PATH_INFO']);
if (!$parsed) {  // Malformed URL
  http_response_code(404);
  exit(1);
}

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
  if (!$file_path) {  // No such racer
    http_response_code(404);
    exit(1);
  }
} else {
  $render = $repo->lookup($parsed['render']);
  if (!$render) {  // No such render
    http_response_code(404);
    exit(1);
  }
  $file_path = $render->find_or_make_image_file($parsed['basename']);
}

header('Pragma: public');
header('Cache-Control: max-age=86400, public');
header('Expires: '.gmdate('D, d M Y H:i:s', time() + 86400).' GMT');

header('Content-type: '.pseudo_mime_content_type($file_path));

readfile($file_path);
