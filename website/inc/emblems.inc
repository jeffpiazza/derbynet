<?php

require_once('inc/path-info.inc');


function all_emblem_image_stems() {
  static $stems = false;
  if ($stems === false) {
    $stems = array();
    for ($century = 0; file_exists(image_file_path("emblem{$century}")); ++$century) {
      $stems[] = "emblem{$century}";
    }
  }
  return $stems;
}

// $i is a zero-based index to select emblem{$i}
function path_to_emblem_image($i) {
  return image_file_path("emblem{$i}");
}
