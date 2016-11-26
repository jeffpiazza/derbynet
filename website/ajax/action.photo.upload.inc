<?php
//     <input type="hidden" name="MAX_FILE_SIZE" value="30000000" />
//     <input name="photo" type="file" />
//     <select name="repo">, options are 'head' or 'car'

//     <input name="racerid">, optional
//     <input name="carnumber">, optional
//          Either of these will assign the image to the indicated racer

require_once('inc/photo-config.inc');

start_response();

// $_FILES['photo']['error']
// $_FILES['photo']['name']
// $_FILES['photo']['size']
// $_FILES['photo']['tmp_name']
// $_FILES['photo']['type']

if (!have_permission(PHOTO_UPLOAD_PERMISSION)) {
  not_authorized_body();
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

    echo '<failure code="upload_'.$photo['error'].'">'.$msg.'</failure>'."\n";
  } else if ($photo['type'] != 'image/png' &&
             $photo['type'] != 'image/jpeg' &&
             $photo['type'] != 'image/gif') {
    echo "<failure code=\"upload_image_type\">Unacceptable file type: ".$photo['type']."</failure>\n";
  } else { 
    $photo_name = basename($photo['name']);
    $target_name = $photo_name;
    $attempt = 1;
    while (file_exists($photo_dir.DIRECTORY_SEPARATOR.$target_name)) {
      $dot = strrpos($photo_name, '.');
      $target_name = substr($photo_name, 0, $dot).'_'.$attempt.substr($photo_name, $dot);
      ++$attempt;
    }
    if ($target_name != $photo_name) {
      echo "\n".'<filename_changed>'.htmlspecialchars($target_name, ENT_QUOTES, 'UTF-8').'</filename_changed>'."\n";
    }

    if (move_uploaded_file($photo['tmp_name'], $photo_dir.DIRECTORY_SEPARATOR.$target_name)) {
      $success = true;

      $racerid = false;
      if (isset($_POST['racerid'])) {
        $racerid = $_POST['racerid'];
      } else if (isset($_POST['carnumber'])) {
        $racerid = read_single_value('SELECT racerid FROM RegistrationInfo'
                                     .' WHERE carnumber = :carno',
                                     array(':carno' => $_POST['carnumber']));
        if (!$racerid) {
          echo "<no-such-car carnumber=\"".$_POST['carnumber']."\"/>\n";
        }
      }

      if ($racerid) {
        $column_name = $photo_repository->column_name();
        // We want to write a value here that will be compatible with what GPRM
        // needs, in case we're working with a GPRM database.
        //
        // For a just-uploaded file, the only file path that will properly exist
        // is the original.  If/when the image actually gets cropped, we'll update
        // the path in the database to point to that.
        $path = $photo_repository->lookup(RENDER_ORIGINAL)->file_path($target_name);
        $success = take_action_silently('UPDATE RegistrationInfo'
                                        .' SET '.$column_name.' = :photo'
                                        .' WHERE racerid = :racerid',
                                        array(':photo' => $path,
                                              ':racerid' => $racerid));
        $row = array('racerid' => $_POST['racerid'],
                     $column_name => $path);
        // This is assuming that there's a 'checkin' render, which isn't the case if we're doing car photos
        echo '<photo-url>'.$photo_repository->url_for_racer($row, RENDER_CHECKIN).'</photo-url>'."\n";

        if (!$success) {
          echo '<failure code="assign_photo">Couldn\'t assign photo to racer.</failure>'."\n";
        }
      }
      if ($success) {
        echo "<success/>\n";
        echo "<uploaded>".htmlspecialchars($target_name, ENT_QUOTES, 'UTF-8')."</uploaded>\n";
        echo "<thumbnail>".htmlspecialchars($photo_repository->lookup(RENDER_THUMBNAIL)->render_url($target_name),
                                            ENT_QUOTES, 'UTF-8')."</thumbnail>\n";
      }
    } else {
      echo '<failure code="move_uploaded">move_uploaded_file failed.';
      if (!is_readable($photo_dir)) {
        echo '  <b>Check settings: photo directory is not readable.</b>';
      }
      if (!is_writable($photo_dir)) {
        echo '  <b>Check settings: photo directory is not writable.</b>';
      } 
      echo '</failure>'."\n";
    }
  }
}
end_response();
?>