<script type="text/javascript">
   // This script should get included by any page that's expected to operate under kiosk.php.
   // Note that $kiosk_page only gets set by kiosk.php, so g_kiosk_page is an empty string when used normally.
   var g_kiosk_page = "<?php echo isset($kiosk_page) ? $kiosk_page : ''; ?>";
   var g_kiosk_poll = Boolean(<?php isset($_GET['page']) ? 0 : 1; ?>);
</script>

