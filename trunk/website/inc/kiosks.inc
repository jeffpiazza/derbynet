<?php

function scan_kiosk_pages($prefix, $kiosk_page) {
  echo '<!-- (('.$kiosk_page.')) -->'."\n";  // TODO
    $dh = opendir(dirname($_SERVER['SCRIPT_FILENAME']).DIRECTORY_SEPARATOR.$prefix);
    while (($entry = readdir($dh)) !== FALSE) {
        if (substr($entry, -6) == ".kiosk") {
            $value = $prefix.DIRECTORY_SEPARATOR.$entry;
            echo '<option '.($kiosk_page == $value ? 'selected="1" ' : '')
            .'value="'.$value.'">'.$entry.'</option>'."\n";
        }
    }
    closedir($dh);
}

function drop_old_kiosks() {
  global $db;
  // TODO current_timestamp is synonym of now(); no need for db_timestamp_expr() ?
  // TODO works for access database?
  // Numeric form of now() is yyyymmddhhmmss, so 1 hour is 10000
  $stmt = $db->query('DELETE FROM Kiosks WHERE '.db_timestamp_expr().' - last_contact > 10000');
}

?>