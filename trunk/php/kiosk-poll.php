<?php session_start(); ?>
<?php
require_once('data.inc');
header('Content-Type: text/xml');
?>
<kiosk_setting page="<?php echo read_raceinfo('kiosk-page'); ?>"/>
<?php
  odbc_close($conn);
?>
