<?php

// This script should get included by any page that's expected to operate
// under kiosk.php.
require_once('inc/ajax-failure.inc');

?>
<script type="text/javascript" src="js/ajax-setup.js"></script>
<script type="text/javascript" src="js/kiosk-poller.js"></script>
<script type="text/javascript">
<?php

if (!isset($_GET['page'])) {
  // We don't poll for new assignments if the kiosk.php URL included a page=
  // parameter (for testing)
  $addr = address_for_current_kiosk();
  $kpage = kiosk_page($addr);
?>
  
$(function() { KioskPoller.start(<?php echo json_encode($addr); ?>,
                                 <?php echo json_encode($kpage['page']); ?>); });

<?php
}  // if
?>
</script>

