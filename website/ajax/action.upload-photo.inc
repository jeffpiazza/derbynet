<?php
//     <input type="hidden" name="MAX_FILE_SIZE" value="30000" />
//     <input name="photo" type="file" />
//     <select name="repo">, options are 'head' or 'car'
//     <input name="racerid">, optional

require_once('inc/photo-config.inc');

// TODO: Permissions!

start_response();

// $_FILES['photo']['error']
// $_FILES['photo']['name']
// $_FILES['photo']['size']
// $_FILES['photo']['tmp_name']
// $_FILES['photo']['type']

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
    if (isset($_POST['racerid'])) {
      $column_name = $photo_repository->column_name();
      $row = array('racerid' => $_POST['racerid'],
                   $column_name => $photo_repository->lookup('display')->file_path($target_name));
      $success = take_action_silently('UPDATE RegistrationInfo'
                                      .' SET '.$column_name.' = :photo'
                                      .' WHERE racerid = :racerid',
                                      array(':photo' => $row[$column_name],
                                            ':racerid' => $row['racerid']));
      echo '<photo-url>'.$photo_repository->url_for_racer($row, 'checkin').'</photo-url>'."\n";

      if (!$success) {
        echo '<failure code="assign_photo">Couldn\'t assign photo to racer.</failure>'."\n";
      }
    }
    if ($success) {
      echo '<success/>'."\n";
    }
  } else {
    echo '<failure code="move_uploaded">move_uploaded_file failed.</failure>'."\n";
  }
}

end_response();
?>