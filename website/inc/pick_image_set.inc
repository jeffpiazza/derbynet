<?php

require_once('inc/path-info.inc');

function emit_images_dir_select($attrs = '') {
  $current = read_raceinfo('images-dir', 'Generic');
  $image_directories = @scandir(image_base_dir());
  if ($image_directories !== false) {
    usort($image_directories,
          function($a, $b) {
            if ($a == 'Generic') return -1;
            if ($b == 'Generic') return 1;
            if ($a < $b) return -1;
            if ($a > $b) return 1;
            return 0; });
    echo "<select $attrs>\n";
    foreach ($image_directories as $img_dir) {
      if ($img_dir[0] == '.') continue;
      $sel = '';
      if ($img_dir == $current) {
        $sel = "selected='selected'";
      }
      echo "<option $sel>".htmlspecialchars($img_dir, ENT_QUOTES, 'UTF-8')."</option>\n";
    }
    echo "</select>\n";
  }
}

?>
