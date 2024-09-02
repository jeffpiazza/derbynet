<?php
// $_POST['slide'] copies an image file to slides directory
// $_POST['subdir']

require_once('inc/path-info.inc');

// Returns true on success; on failure, writes json_failure and returns false.
function perform_test_setup() {
  global $homedir;

  if (isset($homedir)) {
    $slides_dir = $homedir.DIRECTORY_SEPARATOR.'slides';
  } else {
    $slides_dir = data_folder_file_path('slides');
  }

  if (isset($_POST['subdir'])) {
    if (is_acceptable_subdir($_POST['subdir'])) {
      $slides_dir .= DIRECTORY_SEPARATOR.$_POST['subdir'];
    } else {
      json_failure('Invalid subdir');
      return false;
    }
  }

  if (isset($_POST['slide'])) {
    @mkdir($slides_dir);

    json_out('slides_dir', $slides_dir);

    $slide = $_POST['slide'];
    if (!is_acceptable_subdir($slide)) {
      json_out('Invalid slide name');
      return false;
    }

    copy(image_file_path('emblem'), $slides_dir.DIRECTORY_SEPARATOR.$slide);
  } else {
    json_failure('Operation not recognized');
    return false;
  }

  return true;
}

if (perform_test_setup()) {
  json_success();
}

?>
