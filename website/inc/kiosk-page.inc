<?php require_once('inc/ajax-failure.inc'); ?>
<script type="text/javascript" src="js/kiosk-page.js"></script>
<script type="text/javascript">
   // This script should get included by any page that's expected to operate
   // under kiosk.php.
   //
   // Note that $kiosk_page only gets set by kiosk.php, so g_kiosk_page is an
   // empty string when used normally this file is included by a page requested
   // directly.
<?php
   // $kiosk_page is always set unless ?page=
   if (!isset($_GET['page'])) {
     echo '$(function() { kiosk_poll('. json_encode(address_for_current_kiosk()) . ', '
           . json_encode($kiosk_page['page']) . '); })';
   }
?>
</script>

