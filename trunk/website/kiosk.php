<?php
require('inc/data.inc');
$kiosk_page = kiosk_page();
require('kiosks/'.$kiosk_page);
?>