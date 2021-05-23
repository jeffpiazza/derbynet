<?php
// $_POST['directory']

$fullpath = $_POST['directory'].DIRECTORY_SEPARATOR.$_POST['basename'];

if (!have_permission(SET_UP_PERMISSION)) {
  json_not_authorized();
} else if (@mkdir($_POST['directory'])) {
  json_out('path', $_POST['directory']);
  json_success();
} else {
  json_failure('mkdir', "Unable to create directory ".$_POST['directory']);
}

?>