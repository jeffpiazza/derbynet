<?php

require_once('inc/sql-script.inc');

// The FLAGS message from the timer to the server leads to a decode_timer_flags
// call, which populates the TimerSettings table with flags, flag descriptions,
// ports, and timer types (devices) that the timer interface understands.
//
// For flags, changes from the user are written to the 'pending' column for the
// flag entry.
//
// Device selection gets written to a new row with kind='pending-device'.
//
// Port selection sets 'pending' column for the single kind='ports' row.


// If we're working with an unmigrated GPRM database, we may need to create a
// timer settings table.
function create_timer_settings_table() {
  run_sql_script('timer-settings');
}

// Refresh the TimerSettings table with flag values reported from the timer.
function decode_timer_flags($params) {
  global $db;
  try {
    execute_once('DELETE FROM TimerSettings WHERE pending IS NULL');
    foreach ($params as $p => $v) {
      if (substr($p, 0, 5) == "flag-") {
        $vv = explode(':', $v, 2);
        execute_once('INSERT INTO TimerSettings(kind, key, type, value)'
                     .' VALUES(:kind, :key, :type, :value)',
                     array(':kind' => 'flag',
                           ':key' => substr($p, 5),
                           ':type' => $vv[0],
                           ':value' => $vv[1]));
      } else if (substr($p, 0, 5) == "desc-") {
        execute_once('UPDATE TimerSettings SET description = :desc'
                     .' WHERE kind = :kind AND key = :key',
                     array(':kind' => 'flag',
                           ':key' => substr($p, 5),
                           ':desc' => $v));
      } else if ($p == "ports") {
        execute_once('INSERT INTO TimerSettings(kind, key, value)'
                     .' VALUES(:kind, :key, :value)',
                     array(':kind' => 'ports',
                           ':key' => $p,
                           ':value' => $v));
      } else if (substr($p, 0, 7) == "device-") {
        execute_once('INSERT INTO TimerSettings(kind, key, description)'
                     .' VALUES(:kind, :key, :desc)',
                     array(':kind' => 'device',
                           ':key' => substr($p, 7),
                           ':desc' => $v));
      }
    }
  } catch (PDOException $p) {
  }
}

// This gets called as part of the response to each timer-message.  Any pending
// flag values get sent to the timer, and the pending value cleared.  Then we'll
// ask the timer to send a full slate of flags back to the server, in order to
// refresh the TimerSettings table.
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
      execute_once('UPDATE TimerSettings SET pending = NULL');
      execute_once('DELETE FROM TimerSettings WHERE kind = :kind',
                   array(':kind' => 'pending-device'));
      // After sending any changes, ask the timer to send back a FLAGS message
      // to report the full set of flag values, to be recorded in
      // decode_timer_flags.
      echo "  <query/>\n";
    }
  } catch (PDOException $pdo) {
    // Just don't care if the table is missing or broken.
  }
}
?>
