<kiosk.all>
<?php
require_once('inc/kiosks.inc');

echo "<current-scene id=\"".read_raceinfo('current_scene', '')."\"/>\n";

xml_for_all_kiosks();
xml_for_kiosk_pages();
?>
</kiosk.all>