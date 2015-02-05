<?php
require_once('inc/photo-config.inc');

// For rotations, we re-write the original image in place, and generate a new working image.
//
// For cropping, we write a new copy into the cropped directory, and
// re-generate a (cropped) thumbnail image into the thumbs directory.
// Crop numbers are all relative to the working copy, but we do the
// actual cropping by re-reading the original and scaling the cropping
// numbers.

// $_POST: image_name
// usually:
//         left, top, right, bottom, original_height, original_width
//   but alternatively:
//         rotation

// TODO
$image_name = $_POST['image_name'];
$photo_repository = photo_repository(isset($_POST['repo']) ? $_POST['repo'] : 'head');

$original = $photo_repository->lookup('original');
$cropped = $photo_repository->lookup('cropped');

$source_file_path = $original->file_path($image_name);

list($source_width, $source_height, $image_type) = getimagesize($source_file_path);

$im = open_image($source_file_path, $image_type);

if (isset($_POST['rotation']) && $_POST['rotation']) {
  $rot = imagerotate($im, $_POST['rotation'], /* fill color */ 0);
  @unlink($source_file_path);
  if ($rot === false)
    exit(1);
  write_image($rot, $source_file_path, $image_type);
  imagedestroy($im);
  imagedestroy($rot);

  $photo_repository->lookup('original')->delete_dependents($image_name);
  // We're counting on work getting regenerated upon display

  header('Location: photo-crop.php?name='.urlencode($image_name));
  exit(0);
}

$target_file_path = $cropped->target_file_path($image_name);

$x = $_POST['left'] * $source_width / $_POST['original_width'];
$y = $_POST['top'] * $source_height / $_POST['original_height'];
$height = ($_POST['bottom'] - $_POST['top']) * $source_height / $_POST['original_height'];
$width = ($_POST['right'] - $_POST['left']) * $source_width / $_POST['original_width'];
$crop = imagecreatetruecolor($width, $height);
imagecopy($crop, $im, 0,0, $x, $y, $width, $height);

/* PHP 5.5 and up, only!
$crop = imagecrop($im, array('x' => $_POST['left'] * $source_width / $_POST['original_width'],
                             'y' => $_POST['top'] * $source_height / $_POST['original_height'],
                             'height' => ($_POST['bottom'] - $_POST['top']) 
                                 * $source_height / $_POST['original_height'],
                             'width' => ($_POST['right'] - $_POST['left']) 
                                 * $source_width / $_POST['original_width']));
                                 */
write_image($crop, $target_file_path, $image_type);
imagedestroy($crop);
imagedestroy($im);

$cropped->delete_dependents($image_name);
header('Location: photo-thumbs.php');
exit(0);
?>
