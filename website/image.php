<?php
session_start();

// E.g., .../image.php/emblem
// The path_info ("/emblem") provides the image stem name, without any extension.

require_once('inc/data.inc');
require_once('inc/path-info.inc');
require_once('inc/photo-config.inc');


$exploded = explode('/', path_info());
if (count($exploded) == 2) {
  $file_path = image_file_path($exploded[1]);
} else {
  // Don't want this to be a vector for traversing around the server file system
  exit(1);
}


if (is_readable($file_path)) {
  // Cache renewal required every 2 minutes (120 seconds), to get most of the
  // benefit of caching while allowing changing image sets in a "reasonable"
  // amount of time.
  header('Cache-Control: max-age=120, public');
  header('Expires: '.gmdate('D, d M Y H:i:s', time() + 120).' GMT');
  header('Content-type: '.pseudo_mime_content_type($file_path));

  readfile($file_path);
} else {
  header('File-path: '.$file_path);
  header('file_exists: '.file_exists($file_path));
  header('file: '.filesize($file_path));

  echo "Can't read file.";
}

?>
