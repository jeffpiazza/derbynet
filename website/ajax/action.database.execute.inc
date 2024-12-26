<?php
// $_POST['script']
require_once('inc/sql-script.inc');
require_once('inc/locked.inc');
require_once('inc/details-for-setup-page.inc');
require_once('inc/fake_images.inc');
require_once('inc/schema_version.inc');

if (!have_permission(SET_UP_PERMISSION)) {
  json_not_authorized();
} else if (strpbrk($_POST['script'], DIRECTORY_SEPARATOR."/\\") !== false) {
  json_failure('insecure', "Only local SQL script execution is allowed.");
} else {

  try {
    $original_schema = schema_version();
  } catch (PDOException $p) {
  }

  if ($_POST['script'] == 'schema') {
    // Special handling of the "schema" script (reinitializing the database):
    // preserve values for photo-directory and car-photo-directory, if these
    // exist.
    try {
      $saved_raceinfo = save_selected_race_info_values(RACE_INFO_KEYS_FOR_SCHEMA_SCRIPT);
      remove_fake_images($photo_dir);
      remove_fake_images($car_photo_dir);
    } catch (PDOException $p) {
      // If the database has no schema or the wrong schema, don't worry about
      // this.
    }
  }
  
  list($ok, $executed) = run_sql_script($_POST['script']);

  json_out('executed', $executed);

  if (!empty($saved_raceinfo)) {
    restore_saved_race_info_values($saved_raceinfo);
  }

  if ($_POST['script'] == 'update-schema') {
    require_once('inc/update_schema.inc');
    after_update_schema($original_schema);
  }
}

json_out('details', build_setup_page_details());
?>
