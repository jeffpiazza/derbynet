<?php
require('inc/data.inc');
// page query argument to support testing
$kiosk_page = isset($_GET['page']) ? $_GET['page'] : kiosk_page();
require($kiosk_page);
?>