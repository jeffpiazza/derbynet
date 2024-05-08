<?php
session_start(); 

require_once('inc/photo-config.inc');
require_once('inc/path-info.inc');
session_write_close();

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
    if (count($exploded) > 5) {
      return array(
        'repository' => $exploded[1],
        'url_type' => $exploded[2], // 'racer'
        'racerid' => $exploded[3],
        'render' => $exploded[4]);
    }
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

$parsed = parse_photo_url($path_info = path_info());
if (!$parsed) {  // Malformed URL
  http_response_code(404);
  echo "Malformed URL\n";
  // var_dump($path_info);
  exit(1);
}


if ($parsed['url_type'] == 'info') {
  $file_path = read_raceinfo($parsed['key'], $parsed['default']);
} else {
  $repo = photo_repository($parsed['repository']);
  if (!$repo) {  // No such repository
    http_response_code(404);
    echo "No such repository\n";
    // var_dump($parsed);
    exit(1);
  }

  if (isset($parsed['render'])) {
    $render = $repo->lookup($parsed['render']);
    if (!$render) {  // No such render
      http_response_code(404);
      echo "No such render\n";
      // var_dump($parsed);
      exit(1);
    }
  }

  if ($parsed['url_type'] == 'racer') {
    $file_path = read_single_value('SELECT '.$repo->column_name()
                                   .' FROM RegistrationInfo'
                                   .' WHERE racerid = :racerid',
                                   array(':racerid' => $parsed['racerid']));
    if (!$file_path) {
      // Return a 1x1 transparent image if this racer doesn't have an image.
      $file_path = 'img/1x1.png';
    } else if (isset($render)) {
      $file_path = $render->find_or_make_image_file(basename($file_path));
    }
  } else {
    $file_path = $render->find_or_make_image_file($parsed['basename']);
  }
}

if (!$file_path) {  // No such racer/no such photo
  http_response_code(404);
  echo "No file path\n";
  // var_dump($parsed);
  exit(1);
}

header('Pragma: public');
header('Cache-Control: max-age=86400, public');
header('Expires: '.gmdate('D, d M Y H:i:s', time() + 86400).' GMT');

header('Content-type: '.pseudo_mime_content_type($file_path));

readfile($file_path);
?>
