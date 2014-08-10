<?php

function scan_kiosk_pages($prefix, $kiosk_page) {
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

function xml_for_kiosk_pages($prefix) {
  $dh = opendir(dirname($_SERVER['SCRIPT_FILENAME']).DIRECTORY_SEPARATOR.$prefix);
  while (($entry = readdir($dh)) !== FALSE) {
    if (substr($entry, -6) == ".kiosk") {
      $value = $prefix.DIRECTORY_SEPARATOR.$entry;
      echo '<kiosk-page brief="'.htmlspecialchars(substr($entry, 0, -6), ENT_QUOTES, 'UTF-8').'">'
            .htmlspecialchars($value, ENT_QUOTES, 'UTF-8').'</kiosk-page>'."\n";
    }
  }
  closedir($dh);
}

// "old" kiosks are those that haven't made contact in an hour --
// mainly this would be kiosks from testing that we don't want to
// remain in the database on race day.
function drop_old_kiosks() {
  global $db;
  // TODO current_timestamp is synonym of now(); no need for db_timestamp_expr() ?
  // TODO Will timestamps work the same for access database?
  // Numeric form of now() is yyyymmddhhmmss, so 1 hour is 10000
  $stmt = $db->query('DELETE FROM Kiosks WHERE '.db_timestamp_expr().' - last_contact > 10000');
}

?>