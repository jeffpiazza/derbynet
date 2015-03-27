<?php

require_once('inc/sql-script.inc');

function create_kiosk_table() {
  run_sql_script('kiosk-table');
}

function address_for_current_kiosk() {
  if (isset($_SESSION['address'])) {
    $addr = $_SESSION['address'];
  } else {
    $addr = $_SERVER['REMOTE_ADDR'].'+'.$_SERVER['REMOTE_PORT'];
    $_SESSION['address'] = $addr;
  }
  return $addr;
}

// The "more correct" value here would be DIRECTORY_SEPARATOR, but
// apparently the ODBC driver for Access tries to interpret a "\n" as
// a newline character.  That turns "kiosks\now-racing.kiosk" into
// "kiosk \n ow-racing.kiosk", which causes the test in
// js/kiosk-page.js to force a page reload.
define('SEPARATOR', '/');

// Searches the Kiosks table, if there is one, for an entry for the
// current client's IP address plus port.  If there's a page assigned for this
// client, returns that.  Otherwise, returns a RaceInfo entry for the
// default kiosk page, if there is one; otherwise, returns the
// identify kiosk page.
function kiosk_page() {
  global $db;

  $addr = address_for_current_kiosk();

  do {  // Retry loop to allow for having to create the table
    $retry = false;
    try {
      $stmt = $db->prepare('SELECT page, name FROM Kiosks WHERE address = :address');
      $stmt->execute(array(':address' => $addr));
      $row = $stmt->fetch(PDO::FETCH_ASSOC);
      $stmt->closeCursor();
      if ($row) {
        $stmt = $db->prepare('UPDATE Kiosks SET last_contact = '.time().' WHERE address = :address');
        $stmt->execute(array(':address' => $addr));
        if (@$row['page']) {
          return $row;
        }
        // else row exists, but no page set
      } else {
        // No row exists
        $stmt = $db->prepare('INSERT INTO Kiosks(address, page, last_contact)'
                             .' VALUES(:address, :page, '.time().')');
        $stmt->execute(array(':address' => $addr,
                             ':page' => 'kiosks'.SEPARATOR.'identify.kiosk'));
      }
    } catch (PDOException $p) {
      if ($p->getCode() == '42S02') {
        create_kiosk_table();
        $retry = true;
      } else {
        echo $p;
      }
    }
  } while ($retry);
  return array('page' => read_raceinfo('kiosk-page', 'kiosks'.SEPARATOR.'identify.kiosk'));
}

function scan_kiosk_pages($prefix, $kiosk_page) {
  $dh = opendir(dirname($_SERVER['SCRIPT_FILENAME']).SEPARATOR.$prefix);
  if ($dh !== false) {
    while (($entry = readdir($dh)) !== FALSE) {
      if (substr($entry, -6) == ".kiosk") {
        $value = $prefix.SEPARATOR.$entry;
        echo '<option '.($kiosk_page == $value ? 'selected="1" ' : '')
        .'value="'.$value.'">'.$entry.'</option>'."\n";
      }
    }
    closedir($dh);
  }
}

function xml_for_kiosk_pages($prefix) {
  $dh = opendir(dirname($_SERVER['SCRIPT_FILENAME']).SEPARATOR.$prefix);
  if ($dh !== false) {
    while (($entry = readdir($dh)) !== FALSE) {
      if (substr($entry, -6) == ".kiosk") {
        $value = $prefix.SEPARATOR.$entry;
        echo '<kiosk-page brief="'.htmlspecialchars(substr($entry, 0, -6), ENT_QUOTES, 'UTF-8').'">'
        .htmlspecialchars($value, ENT_QUOTES, 'UTF-8').'</kiosk-page>'."\n";
      }
    }
    closedir($dh);
  }
}

// "old" kiosks are those that haven't made contact in five minutes --
// mainly this would be kiosks from testing that we don't want to
// remain in the database on race day.
function drop_old_kiosks() {
  global $db;
  // 300 seconds in five minutes
  $stmt = $db->query('DELETE FROM Kiosks WHERE '.time().' - last_contact > 300');
}
?>