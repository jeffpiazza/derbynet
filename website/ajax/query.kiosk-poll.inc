<?php
require_once('inc/data.inc');
require_once('inc/kiosks.inc');
$kiosk_page = kiosk_page();
?><kiosk_setting page="<?php echo $kiosk_page['page']; ?>" name="<?php echo @$kiosk_page['name']; ?>"/>
