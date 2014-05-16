<?php
session_start(); 

require_once('photo-config.inc');

// Path name is e.g. photo-fetch.php/thumb/mygreatphoto.jpg

$trailing = explode('/', $_SERVER['PATH_INFO']);
$photo_render = $trailing[1];
$image_name = urldecode($trailing[count($trailing) - 1]);

// NOTE: Actually, only thumb and work are used...
if ($photo_render == 'thumb') {
  $target_file_name = resize_to_target($image_name, $photoOriginalsDirectory, $photoThumbsDirectory,
									   $thumbHeight, $thumbWidth);
 } else if ($photo_render == 'work') {
  $target_file_name = resize_to_target($image_name, $photoOriginalsDirectory, $photoWorkDirectory,
									   $workingHeight, $workingWidth);
} else if ($photo_render == 'original') {
  $target_file_name = $photoOriginalsDirectory.'\\'.$image_name;
} else if ($photo_render == 'tiny') {
  $target_file_name = resize_to_target($image_name, $photoOriginalsDirectory, $photoTinyDirectory,
									   $tinyHeight, $tinyWidth);
} else if ($photo_render == 'cropped') {
  $target_file_name = $photoCroppedDirectory.'\\'.$image_name;
  if (!file_exists($target_file_name)) {
	  $target_file_name = $photoOriginalsDirectory.'\\'.$image_name;
  }
} else {
  echo '<h1>Unrecognized render: '.$photo_render.'</h1>'."\n";
  $target_file_name = "";
}

if (file_exists($target_file_name)) {
  header('Pragma: public');
  header('Cache-Control: max-age=86400, public');
  header('Expires: '.gmdate('D, d M Y H:i:s', time() + 86400).' GMT');
  header('Content-type: '.pseudo_mime_content_type($target_file_name));
  readfile($target_file_name);
}