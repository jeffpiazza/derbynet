<?php

require_once('inc/preferences.inc');
require_once('inc/details-for-setup-page.inc');

if (have_permission(SET_UP_PERMISSION)) {
  set_preferences(file_get_contents($_FILES['prefs']['tmp_name']));

  json_out('details', build_setup_page_details());
} else {
  json_not_authorized();
}
?>
