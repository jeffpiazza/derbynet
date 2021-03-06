<?php
// $_POST['script']
require_once('inc/sql-script.inc');
require_once('inc/locked.inc');
require_once('inc/details-for-setup-page.inc');
require_once('inc/fake_images.inc');

if (!have_permission(SET_UP_PERMISSION)) {
  json_not_authorized();
} else if (strpbrk($_POST['script'], DIRECTORY_SEPARATOR."/\\") !== false) {
  json_failure('insecure', "Only local SQL script execution is allowed.");
} else {
  $values_found = false;
  if ($_POST['script'] == 'schema') {
    // Special handling of the "schema" script (reinitializing the database):
    // preserve values for photo-directory and car-photo-directory, if these
    // exist.
    try {
      $photo_dir = read_raceinfo('photo-directory');
      $car_photo_dir = read_raceinfo('car-photo-directory');
      $nlanes = get_lane_count();
      $values_found = true;
      remove_fake_images($photo_dir);
      remove_fake_images($car_photo_dir);
    } catch (PDOException $p) {
      // If the database has no schema or the wrong schema, don't worry about
      // this.
    }
  }
  
  run_sql_script($_POST['script'], /* write_xml */ true);

  if ($values_found) {
    write_raceinfo('photo-directory', $photo_dir);
    write_raceinfo('car-photo-directory', $car_photo_dir);
    if ($nlanes) {
      write_raceinfo('lane_count', $nlanes);
    }
  }

  json_success();
}

json_out('details', build_setup_page_details());
?>