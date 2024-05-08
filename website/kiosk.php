<?php @session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/kiosks.inc');
require_once('inc/permissions.inc');

// The award presentation kiosk wants to reset the current award when it starts
// up.
@$_SESSION['permissions'] |= PRESENT_AWARDS_PERMISSION;

session_write_close();


$g_kiosk_parameters_string = '';
function kiosk_parameters() {
  global $g_kiosk_parameters_string;
  return json_decode(empty($g_kiosk_parameters_string) ? '{}' :  $g_kiosk_parameters_string, true);
}

// 'page' query argument to support testing
if (isset($_GET['page'])) {
  // Confirm the requested page falls under the kiosks directory
  // (Thank you, https://chocapikk.com !)
  $self = dirname(realpath($_SERVER['SCRIPT_FILENAME']));
  $page = realpath($self.'/'.$_GET['page']);
  $kiosks_dir = $self.DIRECTORY_SEPARATOR.'kiosks';
  for ($i = 1; true; ++$i) {
    $dir = dirname($page, $i);
    if ($dir == "/" || $dir == ".") {
      $page = 'kiosks/identify.kiosk';
      break;
    }
    if ($dir == $kiosks_dir) {
      break;
    }
  }
  require($page);
} else {
  $kpage = kiosk_page(address_for_current_kiosk());
  $g_kiosk_parameters_string = $kpage['params'];
  require($kpage['page']);
}
?>
