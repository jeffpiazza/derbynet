<?php
require_once('inc/data.inc');

$photoOriginalsDirectory = photo_directory();
$photoThumbsDirectory = $photoOriginalsDirectory.'\\thumbs';
// "Working size" images, big enough to edit, but small enough to fit on screen
$photoWorkDirectory = $photoOriginalsDirectory.'\\work';
$photoCroppedDirectory = $photoOriginalsDirectory.'\\cropped';
// "Tiny": no more than 80 pixels, because that's what jQuery Mobile
// limits to in a listview.
$photoTinyDirectory = $photoOriginalsDirectory.'\\tiny';

// GPRM Software Options allows for "Max Width" and "Max Height" values.
// photo-size option should match.

$photosize = explode('x', photo_size());
$thumbWidth  =  0 + $photosize[0];
$thumbHeight = 0 + $photosize[1];

$workingWidth  = 950;
$workingHeight = 950;

$tinyWidth = 80;
$tinyHeight = 80;

// Our general strategy for resizing is always to leave the original
// unchanged and write out a new, resized image to a different
// directory using the same base filename.

function resize_to_target($image_name, $from_dir, $to_dir, $height, $width) {
  if (!file_exists($to_dir)) {
	mkdir($to_dir);
  }
  $target_file_name = $to_dir.'\\'.$image_name;
  if (!file_exists($target_file_name)) {
    $source_file_name = $from_dir.'\\'.$image_name;
    $im = new Imagick($source_file_name);
	$im->thumbnailImage($width, $height, /* best fit */ true);
	$im->writeImage($target_file_name);
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
