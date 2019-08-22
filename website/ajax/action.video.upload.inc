<?php
// $_POST['MAX_FILE_SIZE']
//
// $_FILES['video']['error']
// $_FILES['video']['name']
// $_FILES['video']['size']
// $_FILES['video']['tmp_name']
// $_FILES['video']['type']
//
// $_POST['roundid']

start_response();

// TODO Protect against unauthorized uploads
if (false && !have_permission(PHOTO_UPLOAD_PERMISSION)) {
  not_authorized_body();
} else {
  $video_dir = read_raceinfo('video-directory');
  $video = $_FILES['video'];
  
  if ($video['error'] != UPLOAD_ERR_OK) {
    $msg = 'Upload failed.';
    if ($video['error'] == UPLOAD_ERR_INI_SIZE) {
      $msg = 'The uploaded file exceeds the upload_max_filesize directive in php.ini.';
    } else if ($video['error'] == UPLOAD_ERR_FORM_SIZE) {
      $msg = 'The uploaded file exceeds the MAX_FILE_SIZE directive that was specified in the HTML form.';
    } else if ($video['error'] == UPLOAD_ERR_PARTIAL) {
      $msg = 'The uploaded file was only partially uploaded.';
    } else if ($video['error'] == UPLOAD_ERR_NO_FILE) {
      $msg = 'No file was uploaded.';
    } else if ($video['error'] == UPLOAD_ERR_NO_TMP_DIR) {
      $msg = 'Missing a temporary folder.';  // Introduced in PHP 5.0.3.
    } else if ($video['error'] == UPLOAD_ERR_CANT_WRITE) {
      $msg = 'Failed to write file to disk.';  // Introduced in PHP 5.1.0.
    } else if ($video['error'] == UPLOAD_ERR_EXTENSION) {
      $msg = 'A PHP extension stopped the file upload.';
      // PHP does not provide a way to ascertain which extension caused the file
      // upload to stop; examining the list of loaded extensions with phpinfo()
      // may help. Introduced in PHP 5.2.0.
    }

    echo '<failure code="upload_'.$video['error'].'">'.$msg.'</failure>'."\n";
    // } else if ($video['type'] != ...) {
  } else if ($video_dir === false) {
    echo '<failure code="not-configured">No video upload directory configured.</failure>'."\n";
  } else {
    $video_name = basename($video['name']);

    if (file_exists($video_dir.DIRECTORY_SEPARATOR.$video_name)) {
      $dot = strrpos($video_name, '.');
      if ($dot === false) {
        $video_base = $video_name;
        $video_ext = "";
      } else {
        $video_base = substr($video_name, 0, $dot);
        $video_ext = substr($video_name, $dot);
      }

      $i = 1;
      while (file_exists($video_dir.DIRECTORY_SEPARATOR.$video_base.'-'.$i.$video_ext)) {
        ++$i;
      }
      $video_name = $video_base.'-'.$i.$video_ext;
    }

    if (move_uploaded_file($video['tmp_name'], $video_dir.DIRECTORY_SEPARATOR.$video_name)) {
      echo "<success/>\n";
    } else {
      echo "<failure code='move_uploaded'>move_uploaded_file failed.";
      if (!is_readable($video_dir)) {
        echo '  <b>Check settings: video directory is not readable.</b>';
      }
      if (!is_writable($video_dir)) {
        echo '  <b>Check settings: video directory is not writable.</b>';
      }
      echo '</failure>'."\n";
    }
  }
}

end_response();
?>