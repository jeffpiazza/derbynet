<?php

// $_POST['address'] identifies the kiosk to reload

require_once('inc/kiosks.inc');


if (have_permission(CONTROL_RACE_PERMISSION)) {
  mark_kiosk_for_reload($_POST['address']);
} else {
  json_not_authorized();
}
?>