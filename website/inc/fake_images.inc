<?php


function remove_fake_images($dir) {
  if (!is_dir($dir)) {
    return;
  }
  $dh = @opendir($dir);
  if ($dh !== false) {
    while (($filename = readdir($dh)) !== false) {
      if ($filename == '.' || $filename == '..') {
        continue;
      }
      $path = $dir.DIRECTORY_SEPARATOR.$filename;
      if (is_dir($path)) {
        remove_fake_images($path);
      } else if (preg_match("/^fake_[0-9]*\\.svg$/", $filename)) {
        @unlink($path);
      }
    }
    closedir($dh);
  }
}

function remove_fake_car_and_racer_images() {
  remove_fake_images(read_raceinfo('photo-directory'));
  remove_fake_images(read_raceinfo('car-photo-directory'));
}

?>