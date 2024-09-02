<?php
session_start();

// Serve image files out of the Images directory, according to the current images-dir setting.

require_once('inc/data.inc');
session_write_close();
require_once('inc/path-info.inc');
require_once('inc/photo-config.inc');

// path_info (URL) should be:
// slide.php/title   (no extension for title slide)
//   or
// slide.php/my-great-slide.png

$exploded = explode('/', path_info());

if (count($exploded) == 3 && is_acceptable_subdir($exploded[2])) {
  $exploded[1] = $exploded[1].DIRECTORY_SEPARATOR.$exploded[2];
  array_pop($exploded);
}

if (count($exploded) != 2) {
  // Not for probing around the file system
  exit(1);
}

$glob = $exploded[1];
if ($glob == 'title') {
  $glob = 'title.*';
}
$file_path = slide_file_path($glob);


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

  echo "Can't read file ".$file_path;
}

?>
