<?php session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/kiosks.inc');

$g_kiosk_parameters_string = '';
function kiosk_parameters() {
  global $g_kiosk_parameters_string;
  return json_decode($g_kiosk_parameters_string, true);
}

// 'page' query argument to support testing
if (isset($_GET['page'])) {
  require($_GET['page']);
} else {
  $kpage = kiosk_page(address_for_current_kiosk());
  $g_kiosk_parameters_string = $kpage['params'];
  require($kpage['page']);
}
?>