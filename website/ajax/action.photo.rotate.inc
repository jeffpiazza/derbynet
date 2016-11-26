<?php

// $_POST['repo'] - Name of the repository ('head' or 'car')
// $_POST['image_name'] - Base filename of the image
// $_POST['rotation'] - Rotation angle, in degrees COUNTER-clockwise

// TODO Permissions

require_once('inc/photo-config.inc');

start_response();

$image_name = $_POST['image_name'];
$photo_repository = photo_repository(isset($_POST['repo']) ? $_POST['repo'] : 'head');
$rotation = $_POST['rotation'];

$original = $photo_repository->lookup(RENDER_ORIGINAL);

$source_file_path = $original->file_path($image_name);

list($source_width, $source_height, $image_type) = getimagesize($source_file_path);

$im = open_image($source_file_path, $image_type);

$rot = imagerotate($im, $rotation, /* fill color */ 0);

if ($rot !== false) {
  @unlink($source_file_path);
  write_image($rot, $source_file_path, $image_type);

  echo "<success/>\n";
  echo "<cache_breaker time='".time()."'/>\n";
} else {
  echo "<failure code=\"unknown\">Rotation failed.</failure>\n";
}

imagedestroy($im);
imagedestroy($rot);

$photo_repository->lookup(RENDER_ORIGINAL)->delete_dependents($image_name);

end_response();

?>