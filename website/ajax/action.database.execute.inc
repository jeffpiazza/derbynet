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
} else if ($_POST['script'] == 'schema' && locked_settings()) {
  $photo_dir = read_raceinfo('photo-directory');
  $car_photo_dir = read_raceinfo('car-photo-directory');
  
  run_sql_script($_POST['script'], /* write_xml */ true);

  write_raceinfo('photo-directory', $photo_dir);
  write_raceinfo('car-photo-directory', $car_photo_dir);
} else {
  run_sql_script($_POST['script'], /* write_xml */ true);
}

echo "<details><![CDATA[\n";
echo json_encode(build_setup_page_details());
echo "\n]]></details>\n";

end_response();
?>