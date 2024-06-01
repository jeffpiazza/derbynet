<?php
require_once('inc/data.inc');
require_once('inc/kiosks.inc');
require_once('inc/scenes.inc');

maybe_apply_playlist_scene();

if (isset($_GET['address'])) {
  $kiosk_page = kiosk_page($_GET['address']);
}

json_out('kiosk-setting', $kiosk_page);

?>
