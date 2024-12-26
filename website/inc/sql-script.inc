<?php

function sql_file_path($stem) {
  global $dbtype;
  return 'sql'.DIRECTORY_SEPARATOR.$dbtype.DIRECTORY_SEPARATOR.$stem.'.inc';
}

// Returns two values, a bool (true for successful execution) and a list of results
function run_sql_script($scriptname) {
  global $db;
  global $dbtype;
  $ok = true;
  $executed = array();
  try {
    $sql_script = @include(sql_file_path($scriptname));
    if ($sql_script !== false) {
      foreach ($sql_script as $sql_stmt) {
        try {
          $sql_result = $db->exec($sql_stmt);
          if ($sql_result !== false) {
            $executed[] = array('sql' => $sql_stmt, 'result' => $sql_result);
          } else {
            json_sql_failure($sql_stmt);
            $ok = false;
            break;
          }
        } catch (PDOException $p) {
          $executed[] = array('sql' => $sql_stmt, 'result' => $sql_result);

          // Hack to work around the fact that Access doesn't provide a "drop if
          // exists" statement -- you have to attempt a table drop and catch the
          // failure if the table didn't already exist.
          if ($dbtype == 'access' && substr($sql_stmt, 0, 4) == 'DROP') {
          } else {
            json_sql_failure($sql_stmt);
            $ok = false;
            break;
          }
        }
      }
    } else {
      json_failure('sqlnoscript', "SQL script ".@$_POST['script']." not found.");
      $ok = false;
    }
  } catch (PDOException $p) {
    json_failure('sqlfailure', $p->getMessage());
    $ok = false;
  }

  if ($ok) {
    json_success();
  }

  return array($ok, $executed);
}

function save_selected_race_info_values($keys) {
  global $db;
  $saved_raceinfo = array();
  foreach ($db->query('SELECT itemkey, itemvalue FROM RaceInfo') as $row) {
    if (in_array($row['itemkey'], $keys)) {
      $saved_raceinfo[$row['itemkey']] = $row['itemvalue'];
    }
  }
  return $saved_raceinfo;
}

function restore_saved_race_info_values(&$saved_raceinfo) {
  foreach ($saved_raceinfo as $key => $value) {
    write_raceinfo($key, $value);
  }
}

define('RACE_INFO_KEYS_FOR_SCHEMA_SCRIPT',
       ['photo-directory',
        'car-photo-directory',
        'video-directory',
        'logs-directory',
        'lane_count',

        '_websocket_url',
        '_ws_trigger_port']);

define('RACE_INFO_KEYS_FOR_SNAPSHOT',
       ['photo-directory',
        'car-photo-directory',
        'video-directory',
        'logs-directory',

        '_websocket_url',
        '_ws_trigger_port']);
?>
