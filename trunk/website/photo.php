<?php
session_start(); 

require_once('inc/photo-config.inc');

// Path name is e.g. photo.php/thumb/mygreatphoto.jpg
//  or photo.php/thumb/<cache-breaker>/mygreatphoto.jpg

$trailing = explode('/', $_SERVER['PATH_INFO']);
$photo_render = $trailing[1];
$breaker = count($trailing) > 2;
$image_name = urldecode($trailing[count($trailing) - 1]);

$rr = $headshots->lookup($photo_render);
if ($rr) {
  $target_file_path = $rr->find_or_make_image_file($image_name);
} else {
  echo '<h1>Unrecognized render: '.$photo_render.'</h1>'."\n";
  exit(0); // $target_file_path = "";
}

if ($breaker) {
  header('Pragma: public');
  header('Cache-Control: max-age=86400, public');
  header('Expires: '.gmdate('D, d M Y H:i:s', time() + 86400).' GMT');
}

header('Content-type: '.pseudo_mime_content_type($target_file_path));

readfile($target_file_path);
