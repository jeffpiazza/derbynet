<?php
require('inc/data.inc');

// 'page' query argument to support testing
if (isset($_GET['page'])) {
  require($_GET['page']);
} else {
  $kiosk_page = kiosk_page();
  require($kiosk_page['page']);
}
?>