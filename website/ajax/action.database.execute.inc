<?php
// $_POST['script']
require_once('inc/sql-script.inc');
require_once('inc/locked.inc');
require_once('inc/details-for-setup-page.inc');

start_response();

if (!have_permission(SET_UP_PERMISSION)) {
  not_authorized_body();
} else if (strpbrk($_POST['script'], DIRECTORY_SEPARATOR."/\\") !== false) {
  echo "<failure code='insecure'>Only local SQL script execution is allowed.</failure>\n";
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
}

echo "<details><![CDATA[\n";
echo json_encode(build_setup_page_details());
echo "\n]]></details>\n";

end_response();
?>