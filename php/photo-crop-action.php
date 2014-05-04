<?php
require_once('photo-config.inc');

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

$image_name = $_POST['image_name'];
$from_dir = $photoOriginalsDirectory;
$source_file_name = $from_dir.'\\'.$image_name;
$im = new Imagick($source_file_name);

if (isset($_POST['rotation']) && $_POST['rotation']) {
  $im->rotateImage(new ImagickPixel('none'), $_POST['rotation']);
  @unlink($source_file_name);
  $im->writeImage();

  @unlink($photoThumbsDirectory.'\\'.$image_name);
  @unlink($photoTinyDirectory.'\\'.$image_name);
  @unlink($photoWorkDirectory.'\\'.$image_name);
  // Re-write working image.  This will give a new file date, so stale
  // cache image won't be used.
  resize_to_target($image_name, $photoOriginalsDirectory, $photoWorkDirectory,
				   $workingHeight, $workingWidth);
  header('Location: photo-crop.php?name='.urlencode($image_name));
  exit(0);
}


$to_dir = $photoCroppedDirectory;

if (!file_exists($to_dir)) {
  mkdir($to_dir);
}

$target_file_name = $to_dir.'\\'.$image_name;

$im->cropImage(($_POST['right'] - $_POST['left']) * $im->getImageWidth() / $_POST['original_width'],
			   ($_POST['bottom'] - $_POST['top']) * $im->getImageHeight() / $_POST['original_height'],
			   $_POST['left'] * $im->getImageWidth() / $_POST['original_width'],
			   $_POST['top'] * $im->getImageHeight() / $_POST['original_height']);
$im->writeImage($target_file_name);

@unlink($photoThumbsDirectory.'\\'.$image_name);
@unlink($photoTinyDirectory.'\\'.$image_name);
resize_to_target($image_name, $to_dir, $photoThumbsDirectory, $thumbHeight, $thumbWidth);
resize_to_target($image_name, $to_dir, $photoTinyDirectory, $tinyHeight, $tinyWidth);
header('Location: photo-thumbs.php');
exit(0);
?>
