<?php

$php_file = $argv[1];
$directory = $argv[2];  // e.g., 'data/headshots';

require_once($php_file);

$start = microtime(true);
$detector = new FaceDetector();

$files = array();

$dh = @opendir($directory);
while (($filename = readdir($dh)) !== false) {
  if (preg_match('/jpg/i', $filename) && is_file($directory.DIRECTORY_SEPARATOR.$filename)) {
    $files[] = $directory.DIRECTORY_SEPARATOR.$filename;
  }
}
closedir($dh);

echo count($files)." photos to scan.\n";
$fail_count = 0;
foreach ($files as $file) {
  $image = imagecreatefromjpeg($file);
  $detection_start_time = microtime(true);
  $box = $detector->detect_face($image);
  $detection_end_time = microtime(true);
  imagedestroy($image);
  if ($box) {
    echo "   Identified ".$file." in ".($detection_end_time - $detection_start_time)."s\n";
  } else {
    ++$fail_count;
    echo "** Failed for ".$file." in ".($detection_end_time - $detection_start_time)."s\n";
  }
}
$end_time = microtime(true);
echo "Net: $fail_count failed out of ".count($files)."\n";
echo "Total time ".($end_time - $start)."s\n";

?>