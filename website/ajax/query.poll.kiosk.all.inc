<kiosk.all>
<?php
require_once('inc/kiosks.inc');
try {
  drop_old_kiosks();
  $stmt = $db->query('SELECT address, name, page, last_contact'
                     .' FROM Kiosks'
                     .' ORDER BY name, address, last_contact');
  foreach ($stmt as $row) {
    parse_kiosk_params($row);
    echo '<kiosk>'."\n";
    echo '  <name>'.htmlspecialchars($row['name'], ENT_QUOTES, 'UTF-8').'</name>'."\n";
    echo '  <address>'.htmlspecialchars($row['address'], ENT_QUOTES, 'UTF-8').'</address>'."\n";
    echo '  <last_contact>'.floor((time() - $row['last_contact'])/60)
         .' minute(s) ago</last_contact>'."\n";
    echo '  <assigned_page>'.htmlspecialchars($row['page'], ENT_QUOTES, 'UTF-8').'</assigned_page>'."\n";
    echo '  <parameters>'.htmlspecialchars($row['params'], ENT_QUOTES, 'UTF-8').'</parameters>'."\n";
    echo '</kiosk>'."\n";
    }
} catch (PDOException $p) {
  // Most likely: no kiosks table.
  // One will be created if a kiosk actually connects.
}

xml_for_kiosk_pages('kiosks');
xml_for_kiosk_pages('local'.DIRECTORY_SEPARATOR.'kiosks');
?>
</kiosk.all>