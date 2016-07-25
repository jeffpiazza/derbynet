<?php require_once('inc/ajax-failure.inc'); ?>
<script type="text/javascript" src="js/kiosk-page.js"></script>
<script type="text/javascript">
<?php
   // This script should get included by any page that's expected to operate
   // under kiosk.php.
   if (!isset($_GET['page'])) {
     echo '$(function() { kiosk_poll('. json_encode(address_for_current_kiosk()) . ', '
           . json_encode($kiosk_page['page']) . '); })';
   }
?>
</script>

