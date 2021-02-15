<?php


require_once('inc/sql-script.inc');

// If we're working with an unmigrated GPRM database, we may need to create a
// kiosk table.
function create_timer_settings_table() {
  run_sql_script('timer-settings');
}

function decode_timer_flags($params) {
  global $db;
  try {
    read_single_value('DELETE FROM TimerSettings WHERE pending IS NULL');
    foreach ($params as $p => $v) {
      if (substr($p, 0, 5) == "flag-") {
        $vv = explode(':', $v, 2);
        read_single_value('INSERT INTO TimerSettings(kind, key, type, value)'
                          .' VALUES(:kind, :key, :type, :value)',
                          array(':kind' => 'flag',
                                ':key' => substr($p, 5),
                                ':type' => $vv[0],
                                ':value' => $vv[1]));
      } else if (substr($p, 0, 5) == "desc-") {
        read_single_value('UPDATE TimerSettings SET description = :desc'
                          .' WHERE kind = :kind AND key = :key',
                          array(':kind' => 'flag',
                                ':key' => substr($p, 5),
                                ':desc' => $v));
      } else if ($p == "ports") {
        read_single_value('INSERT INTO TimerSettings(kind, key, value)'
                          .' VALUES(:kind, :key, :value)',
                          array(':kind' => 'ports',
                                ':key' => $p,
                                ':value' => $v));
      } else if (substr($p, 0, 7) == "device-") {
        read_single_value('INSERT INTO TimerSettings(kind, key, description)'
                          .' VALUES(:kind, :key, :desc)',
                          array(':kind' => 'device',
                                ':key' => substr($p, 7),
                                ':desc' => $v));
      }
    }
  } catch (PDOException $p) {
  }
}

function send_timer_assignments() {
  global $db;
  try {
    $stmt = $db->prepare('SELECT kind, key, pending FROM TimerSettings'
                         .' WHERE pending IS NOT NULL');
    $stmt->execute(array());
    $any_assigned = false;
    foreach ($stmt as $row) {
      $any_assigned = true;
      if ($row['kind'] == 'flag') {
        echo "<assign-flag flag=\"$row[key]\" value=\"";
        echo htmlspecialchars($row['pending'], ENT_QUOTES, 'UTF-8');
        echo "\"/>\n";
      } else if ($row['kind'] == 'ports') {
        echo "<assign-port";
        if ($row['pending'] != '') {
          echo " port=\"";
          echo htmlspecialchars($row['pending'], ENT_QUOTES, 'UTF-8');
          echo "\"";
        }
        echo "/>\n";
      } else if ($row['kind'] == 'pending-device') {
        echo "<assign-device";
        if ($row['pending'] != '') {
          echo " device=\"";
          echo htmlspecialchars($row['pending'], ENT_QUOTES, 'UTF-8');
          echo "\"";
        }
        echo "/>\n";
      }
    }
    if ($any_assigned) {
      read_single_value('UPDATE TimerSettings SET pending = NULL');
      read_single_value('DELETE FROM TimerSettings WHERE kind = :kind',
                        array(':kind' => 'pending-device'));
      echo "  <query/>\n";
    }
  } catch (PDOException $pdo) {
    // Just don't care if the table is missing or broken.
  }
}
?>