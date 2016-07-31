<?php require_once('inc/ajax-failure.inc'); ?>
<script type="text/javascript" src="js/kiosk-page.js"></script>
<script type="text/javascript">
<?php
   // This script should get included by any page that's expected to operate
   // under kiosk.php.
   if (!isset($_GET['page'])) {
     $addr = address_for_current_kiosk();
     $kpage = kiosk_page($addr);
     echo '$(function() { kiosk_poll('. json_encode($addr) . ', '
           . json_encode($kpage['page']) . '); })';
   }
?>
</script>

