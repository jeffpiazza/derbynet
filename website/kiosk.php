<?php session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/kiosks.inc');

// 'page' query argument to support testing
if (isset($_GET['page'])) {
  require($_GET['page']);
} else {
  $kpage = kiosk_page(address_for_current_kiosk());
  // For kiosk pages that use parameters:
  $params = $kpage['params'];
  require($kpage['page']);
}
?>