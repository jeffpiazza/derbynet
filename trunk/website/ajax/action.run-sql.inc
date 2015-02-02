<?php
// $_POST['script']
require_once('inc/sql-script.inc');

start_response();

if (have_permission(SET_UP_PERMISSION)) {
  run_sql_script($_POST['script'], /* write_xml */ true);
} else {
  not_authorized_body();
}

end_response();
?>