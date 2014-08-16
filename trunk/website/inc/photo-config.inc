<?php
require_once('inc/data.inc');

$photoOriginalsDirectory = photo_directory();
$photoThumbsDirectory = $photoOriginalsDirectory.DIRECTORY_SEPARATOR.'thumbs';
// "Working size" images, big enough to edit, but small enough to fit on screen
$photoWorkDirectory = $photoOriginalsDirectory.DIRECTORY_SEPARATOR.'work';
$photoCroppedDirectory = $photoOriginalsDirectory.DIRECTORY_SEPARATOR.'cropped';
// "Tiny": no more than 80 pixels, because that's what jQuery Mobile
// limits to in a listview.
$photoTinyDirectory = $photoOriginalsDirectory.DIRECTORY_SEPARATOR.'tiny';

// GPRM Software Options allows for "Max Width" and "Max Height" values.
// photo-size option should match.

$photosize = explode('x', photo_size());
$thumbWidth  =  0 + $photosize[0];
$thumbHeight = 0 + $photosize[1];

$workingWidth  = 950;
$workingHeight = 950;

$tinyWidth = 80;
$tinyHeight = 80;

function open_image($filepath, $image_type) {
  switch ($image_type) {
  case IMAGETYPE_GIF:
    return imagecreatefromgif($filepath);
  case IMAGETYPE_JPEG:
    return imagecreatefromjpeg($filepath);
  case IMAGETYPE_PNG:
    return imagecreatefrompng($filepath);
  }

  return FALSE;
}

function write_image($image, $filepath, $image_type) {
  switch ($image_type) {
  case IMAGETYPE_GIF:
    return imagegif($image, $filepath);
  case IMAGETYPE_JPEG:
    return imagejpeg($image, $filepath, 90);
  case IMAGETYPE_PNG:
    return imagepng($image, $filepath, 0);
  }
  return FALSE;
}

// Our general strategy for resizing is always to leave the original
// unchanged and write out a new, resized image to a different
// directory using the same base filename.

function resize_to_target($image_name, $from_dir, $to_dir, $height, $width) {
  if (!file_exists($to_dir)) {
	mkdir($to_dir);
  }
  $target_file_name = $to_dir.DIRECTORY_SEPARATOR.$image_name;
  if (!file_exists($target_file_name)) {
    $source_file_path = $from_dir.DIRECTORY_SEPARATOR.$image_name;

    list($source_width, $source_height, $image_type) = getimagesize($source_file_path);
    if ($source_height == 0) {
      return FALSE;
    }

    $im = open_image($source_file_path, $image_type);

    $aspect = $source_width / $source_height;

    if ($aspect > $width / $height) {
      // At any height, source image is too wide, so pick scaling to
      // convert $source_width to $width.  The scale factor is then
      // $width/$source_width.
      $height = (int) ($source_height * $width / $source_width);
    } else {
      // Conversely, scale to height.
      $width = (int) ($source_width * $height / $source_height);
    }

    $thumb = imagecreatetruecolor($width, $height);
    imagecopyresampled($thumb, $im, 0,0,0,0, $width, $height, $source_width, $source_height);

    write_image($thumb, $target_file_name, $image_type);

    imagedestroy($im);
    imagedestroy($thumb);
  }
  return $target_file_name;
}

function pseudo_mime_content_type($filename){
  $mime_types = array(
            'txt' => 'text/plain',
            'htm' => 'text/html',
            'html' => 'text/html',
            'php' => 'text/html',
            'css' => 'text/css',
            'js' => 'application/javascript',
            'json' => 'application/json',
            'xml' => 'application/xml',
            'swf' => 'application/x-shockwave-flash',
            'flv' => 'video/x-flv',

            // images
            'png' => 'image/png',
            'jpe' => 'image/jpeg',
            'jpeg' => 'image/jpeg',
            'jpg' => 'image/jpeg',
            'gif' => 'image/gif',
            'bmp' => 'image/bmp',
            'ico' => 'image/vnd.microsoft.icon',
            'tiff' => 'image/tiff',
            'tif' => 'image/tiff',
            'svg' => 'image/svg+xml',
            'svgz' => 'image/svg+xml',

            // archives
            'zip' => 'application/zip',
            'rar' => 'application/x-rar-compressed',
            'exe' => 'application/x-msdownload',
            'msi' => 'application/x-msdownload',
            'cab' => 'application/vnd.ms-cab-compressed',

            // audio/video
            'mp3' => 'audio/mpeg',
            'qt' => 'video/quicktime',
            'mov' => 'video/quicktime',

            // adobe
            'pdf' => 'application/pdf',
            'psd' => 'image/vnd.adobe.photoshop',
            'ai' => 'application/postscript',
            'eps' => 'application/postscript',
            'ps' => 'application/postscript',

            // ms office
            'doc' => 'application/msword',
            'rtf' => 'application/rtf',
            'xls' => 'application/vnd.ms-excel',
            'ppt' => 'application/vnd.ms-powerpoint',

            // open office
            'odt' => 'application/vnd.oasis.opendocument.text',
            'ods' => 'application/vnd.oasis.opendocument.spreadsheet',
        );

  $ext = strtolower(array_pop(explode('.',$filename)));
  if (array_key_exists($ext, $mime_types)) {
	return $mime_types[$ext];
  }
  elseif (function_exists('finfo_open')) {
	$finfo = finfo_open(FILEINFO_MIME);
	$mimetype = finfo_file($finfo, $filename);
	finfo_close($finfo);
	return $mimetype;
  }
  else {
	return 'application/octet-stream';
  }
}
?>
