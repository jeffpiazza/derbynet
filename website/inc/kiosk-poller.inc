<?php

// This script should get included by any page that's expected to operate
// under kiosk.php.

?>
<script type="text/javascript" src="js/ajax-setup.js"></script>
<script type="text/javascript" src="js/kiosk-poller.js"></script>
<script type="text/javascript">
<?php

if (!isset($_GET['page'])) {
  // $g_kiosk_address is assigned in kiosk.php
  $kpage = kiosk_page($g_kiosk_address);
?>
  
$(function() { KioskPoller.start(<?php echo json_encode($g_kiosk_address); ?>,
                                 <?php echo json_encode($kpage['page']); ?>); });

<?php
}  // if
?>
</script>

