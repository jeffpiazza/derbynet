<?php
// $_POST['MAX_FILE_SIZE']
//
// $_FILES['photo']['error']
// $_FILES['photo']['name']
// $_FILES['photo']['size']
// $_FILES['photo']['tmp_name']
// $_FILES['photo']['type']
//
// $_POST['repo'], options are 'head' or 'car'
//
// $_POST['racerid'], optional
// $_POST['carnumber'], optional
// $_POST['barcode'], optional
//          Any of these will assign the image to the indicated racer
//
// $_POST['autocrop']: if present, and if facedetection code is available, try
// to identify the face and crop the photo to a head shot.

require_once('inc/photo-config.inc');
require_once('inc/barcode.inc');
require_once('inc/events.inc');

function widen_and_clip(&$v, &$width, $scale, $max) {
  $original_width = $width;
  $width = round($width * $scale);
  $v = $v - round(($width - $original_width) / 2);

  if ($v < 0) {
    $width = $width + $v;  // $width - (-$v)
    $v = 0;  // $v + (-$v)
  }

  if ($v + $width > $max) {
    $width = $max - $v;
  }
}

if (!have_permission(PHOTO_UPLOAD_PERMISSION)) {
  json_not_authorized();
} else {
  $photo = $_FILES['photo'];
  $photo_repository = photo_repository(isset($_POST['repo']) ? $_POST['repo'] : 'head');
  $photo_dir = $photo_repository->directory();

  if ($photo['error'] != UPLOAD_ERR_OK) {
    $msg = 'Upload failed.';
    if ($photo['error'] == UPLOAD_ERR_INI_SIZE) {
      $msg = 'The uploaded file exceeds the upload_max_filesize directive in php.ini.';
    } else if ($photo['error'] == UPLOAD_ERR_FORM_SIZE) {
      $msg = 'The uploaded file exceeds the MAX_FILE_SIZE directive that was specified in the HTML form.';
    } else if ($photo['error'] == UPLOAD_ERR_PARTIAL) {
      $msg = 'The uploaded file was only partially uploaded.';
    } else if ($photo['error'] == UPLOAD_ERR_NO_FILE) {
      $msg = 'No file was uploaded.';
    } else if ($photo['error'] == UPLOAD_ERR_NO_TMP_DIR) {
      $msg = 'Missing a temporary folder.';  // Introduced in PHP 5.0.3.
    } else if ($photo['error'] == UPLOAD_ERR_CANT_WRITE) {
      $msg = 'Failed to write file to disk.';  // Introduced in PHP 5.1.0.
    } else if ($photo['error'] == UPLOAD_ERR_EXTENSION) {
      $msg = 'A PHP extension stopped the file upload.';
      // PHP does not provide a way to ascertain which extension caused the file
      // upload to stop; examining the list of loaded extensions with phpinfo()
      // may help. Introduced in PHP 5.2.0.
    }

    json_failure('upload_'.$photo['error'], $msg);
  } else if ($photo['type'] != 'image/png' &&
             $photo['type'] != 'image/jpeg' &&
             $photo['type'] != 'image/gif') {
    json_failure('upload_image_type', "Unacceptable file type: ".$photo['type']);
  } else {
    $photo_name = basename(urldecode($photo['name']));
    // Filter out special characters from the filename
    $photo_name = preg_replace("/[^-a-zA-Z0-9 .]+/", "_", $photo_name);
    $target_name = $photo_name;
    $attempt = 1;
    while (file_exists($photo_dir.DIRECTORY_SEPARATOR.$target_name)) {
      $dot = strrpos($photo_name, '.');
      $target_name = substr($photo_name, 0, $dot).'_'.$attempt.substr($photo_name, $dot);
      ++$attempt;
    }
    if ($target_name != $photo_name) {
      json_out('filename-change', $target);
    }

    if (move_uploaded_file($photo['tmp_name'], $photo_dir.DIRECTORY_SEPARATOR.$target_name)) {
      $success = true;

      // For a just-uploaded file, the only file path that will properly exist
      // is the original.  If/when the image gets cropped, either in this action
      // or a subsequent one, we'll update the path in the database to point to
      // that.
      $path_render = $photo_repository->lookup(RENDER_ORIGINAL);

      if (isset($_POST['autocrop']) && $_POST['autocrop']) {
        try {
          $source_file_path = $photo_dir.DIRECTORY_SEPARATOR.$target_name;
          list($source_width, $source_height, $image_type) = getimagesize($source_file_path);

          $original = open_image($source_file_path, $image_type);

          if (isset($_SERVER['DERBYNET_FACEDETECT']) && is_executable($_SERVER['DERBYNET_FACEDETECT'])) {
            $output = false;
            $coords = @exec($_SERVER['DERBYNET_FACEDETECT'].' \''.$source_file_path.'\'', $output);

            if ($coords !== false) {
              $coords = explode(' ', $coords);
              if (count($coords) == 4) {
                $face = array('x' => $coords[0],
                              'y' => $coords[1],
                              'width' => $coords[2],
                              'height' => $coords[3]);
              }
            }
          } else {
            require_once('inc/facedetection.inc');
            $detector = new FaceDetector();
            $face = $detector->detect_face($original);
          }

          if ($face) {
            $x = round($face['x']);
            $y = round($face['y']);
            $w = round($face['width']);
            // Relying on face detection returning a square, with width = height

            // The marked face is a little tight for cropping -- widen by 10%,
            // then adjust to 4:3 aspect
            widen_and_clip($x, $w, 1.10, imagesx($original));
            $h = $w;
            widen_and_clip($y, $h, 4 / 3, imagesy($original));

            if ($w != 0 && $h != 0) {
              $cropped_path = $photo_repository->lookup(RENDER_CROPPED)->target_file_path($target_name);
              $cropped = imagecreatetruecolor($w, $h);
              imagecopy($cropped, $original, 0,0, $x, $y, $w, $h);
              write_image($cropped, $cropped_path, $image_type);
              imagedestroy($cropped);
              // Use the cropped path now rather than the original if we record
              // the file name in the database.
              $path_render = $photo_repository->lookup(RENDER_CROPPED);
              json_out('cropped', true);
            } else {
              json_out('warning', "Autocrop gave $w width x $h height");
            }
          } else {
            json_out('warning', "No face detected for autocrop.");
          }
          imagedestroy($original);
        } catch (Exception $e) {
          json_out('warning', $e->getMessage());
        }
      }

      $racerid = false;
      if (isset($_POST['racerid'])) {
        $racerid = $_POST['racerid'];
      } else if (isset($_POST['carnumber'])) {
        $racerid = read_single_value('SELECT racerid FROM RegistrationInfo'
                                     .' WHERE carnumber = :carno',
                                     array(':carno' => $_POST['carnumber']));
        if (!$racerid) {
          json_failure('no-such-car', "No car numbered ".$_POST['carnumber']." found.");
          $success = false;
        }
      } else if (isset($_POST['barcode'])) {
        if ($_POST['barcode'] == 'PWDuploadtest') {
          // For an upload test, we delete the photo after uploading, and report success.
          @unlink($photo_dir.DIRECTORY_SEPARATOR.$target_name);
          @unlink($cropped_path = $photo_repository->lookup(RENDER_CROPPED)->target_file_path($target_name));
        } else if ($_POST['barcode'] == 'PWDuploadonly') {
          // Upload photo without associating with a racer
        } else {
          $racerid = barcode_to_racerid($_POST['barcode']);
          if (!$racerid) {
            json_failure('barcode', "Barcode ".$_POST['barcode']." not recognized.");
            $success = false;
          }
        }
      }

      if ($racerid) {
        $column_name = $photo_repository->column_name();
        // We want to write a value here that will be compatible with what GPRM
        // needs, in case we're working with a GPRM database.
        $path = $path_render->file_path($target_name);
        $success = take_action_silently('UPDATE RegistrationInfo'
                                        .' SET '.$column_name.' = :photo'
                                        .' WHERE racerid = :racerid',
                                        array(':photo' => $path,
                                              ':racerid' => $racerid));
        record_event($photo_repository->name() == 'head' ? EVENT_RACER_PHOTO_ASSIGN : EVENT_CAR_PHOTO_ASSIGN,
                     array('racerid' => $racerid));

        $row = array('racerid' => $racerid,
                     $column_name => $path);
        // This is assuming that there's a 'checkin' render, which isn't the case if we're doing car photos
        json_out('photo-url', $photo_repository->url_for_racer($row, RENDER_CHECKIN));

        if (!$success) {
          json_failure('assign_photo', "Couldn\'t assign photo to racer.");
        }
      }

      if ($success) {
        json_out('uploaded', $target_name);
        json_out('thumbnail', $photo_repository->lookup(RENDER_THUMBNAIL)->render_url($target_name));
        json_success();
      }
    } else {
      json_failure('move_uploaded',
                   "move_uploaded_file failed."
                   .(is_readable($photo_dir) ? '' :
                     '  <b>Check settings: photo directory is not readable.</b>')
                   .(is_writable($photo_dir) ? '' :
                     '  <b>Check settings: photo directory is not writable.</b>'));
    }
  }
}
?>
