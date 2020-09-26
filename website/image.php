<?php
session_start();

// Serve image files out of the Images directory, according to the current images-dir setting.

require_once('inc/data.inc');
require_once('inc/path-info.inc');
require_once('inc/photo-config.inc');

// path_info (URL) should be:
// image.php/filename

$exploded = explode('/', path_info());
if (count($exploded) == 2) {
  $file_path = image_file_path($exploded[1]);

  if (is_readable($file_path)) {
    header('Pragma: public');
    header('Cache-Control: max-age=86400, public');
    header('Expires: '.gmdate('D, d M Y H:i:s', time() + 86400).' GMT');
    header('Content-type: '.pseudo_mime_content_type($file_path));

    readfile($file_path);
  } else {
    header('File-path: '.$file_path);
    header('file_exists: '.file_exists($file_path));
    header('file: '.filesize($file_path));

    echo "Can't read file.";
  }
} else {
  exit(1);
}

?>