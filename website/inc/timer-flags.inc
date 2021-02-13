<?php


require_once('inc/sql-script.inc');

// If we're working with an unmigrated GPRM database, we may need to create a
// kiosk table.
function create_timer_settings_table() {
  run_sql_script('timer-settings');
}


function decode_timer_flags($params) {
  global $db;
  @create_timer_settings_table();  // Drop and recreate the table each time
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
}

?>