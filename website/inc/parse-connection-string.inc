<?php

require_once('inc/default-file-path.inc');
require_once('inc/schema_version.inc');

// Returns an array whose keys will include:
//
// connection_string
// scheme        one of: 'sqlite', 'mysql', 'odbc', or 'unknown'
//
// ... and, depending on scheme, might also include:
//
// sqlite_path
// mysql_host
// mysql_dbname
// odbc_dsn_name
function parse_connection_string($conn_string) {
  // If there's an existing database config, then $conn_string may
  // contain the connection string, which we can parse to better populate the form
  // fields.
  $result = array();

  if (isset($conn_string) && $conn_string) {
    $result['connection_string'] = $conn_string;
    if (substr($conn_string, 0, 7) == 'sqlite:') {
      $result['scheme'] = 'sqlite';
      $result['sqlite_path'] = substr($conn_string, 7);
    } else if (substr($conn_string, 0, 6) == 'mysql:') {
      $result['scheme'] = 'mysql';
      if (preg_match('/host=([^;]*);/', $conn_string, $matches)) {
        $result['mysql_host'] = $matches[1];
      } else {
        $result['mysql_host'] = '';
      }
      if (preg_match('/dbname=(.*)/', $conn_string, $matches)) {
        $result['mysql_dbname'] = $matches[1];
      } else {
        $result['mysql_dbname'] = '';
      }
    } else if (substr($conn_string, 0, 5) == 'odbc:') {
      if (preg_match('/DSN=([^;]*);/', $conn_string, $matches)) {
        $result['scheme'] = 'odbc';
        $result['odbc_dsn_name'] = $matches[1];
      } else {
        $result['scheme'] = 'unknown';
      }
    } else {
      $result['scheme'] = 'unknown';
    }
  } else {
    $result['scheme'] = 'empty';
    $result['connection_string'] = '';
  }

  return $result;
}

// Tests whether there are any tables defined in the database, for those
// database types that we know how to test.
function database_is_empty() {
  global $db;
  global $dbtype;
  if (isset($db) && isset($dbtype)) {
    try {
      if ($dbtype == 'sqlite') {
        if (read_single_value("SELECT COUNT(*) FROM sqlite_master"
                              ." WHERE type='table'", array()) == 0) {
          return true;
        }
      } else if ($dbtype == 'mysql') {
        if (read_single_value("SELECT COUNT(DISTINCT table_name) FROM information_schema.columns"
                              ." WHERE table_schema = database()", array()) == 0) {
          return true;
        }
      }
    } catch (PDOException $p) {
    }
  }

  return false;
}

function connection_string_to_html($conn_string) {
  $parsed = parse_connection_string($conn_string);
  if ($parsed['scheme'] == 'sqlite') {
    return "SQLITE: ".htmlspecialchars($parsed['sqlite_path'], ENT_QUOTES, 'UTF-8');
  } else if ($parsed['scheme'] == 'mysql') {
    return "MYSQL: Database <b>".htmlspecialchars($parsed['mysql_dbname'], ENT_QUOTES, 'UTF-8')."</b>"
           ." on ".htmlspecialchars($parsed['mysql_host'], ENT_QUOTES, 'UTF-8');
  } else if ($parsed['scheme'] == 'odbc') {
    return "ODBC: ".htmlspecialchars($parsed['odbc_dsn_name'], ENT_QUOTES, 'UTF-8');
  } else {
    return htmlspecialchars($parsed['connection_string'], ENT_QUOTES, 'UTF-8');
  }
}

function pad3($n) {
  return str_pad($n, 3, '0', STR_PAD_LEFT);
}

// Assumes $db has already been defined if $db_connection_string is available
// The presence of $db implies that $db_connection_string identifies an actual
// database (e.g., for sqlite, the file is present)
function inspect_database() {
  global $db;
  // Initialize with default values
  $result = array('status' => 'trouble',  // icon for schema step
                  // What the button for schema step should say.  The special value 'disabled'
                  // means the button should be disabled (because the database isn't defined).
                  'button' => 'disabled',
                  'message' => "<p><span class='trouble'>Schema not available without a database; "
                               ."configure the database first.</span></p>",
                  'group_label' => 'Den',
                  'group_label_lc' => 'den',
                  'nracers' => 0,
                  'dens' => array(),
                  'nawards' => 0,
                  'settings_icon' => 'unknown',
                  'settings_details' => 'No photo directories defined.',
                  'writable' => false
    );

  if (isset($db) && $db) {
    try {
      $schema_version = schema_version();
      $result['group_label'] = group_label();
      $result['group_label_lc'] = group_label_lc();
      
      $result['nracers'] = read_single_value('SELECT COUNT(*) FROM RegistrationInfo', array());
      $result['nawards'] = read_single_value('SELECT COUNT(*) FROM Awards', array());

      $sorting_field = 'sortorder';
      if ($schema_version < 2) { $sorting_field = 'class'; }

      $dens = array();
      foreach ($db->query('SELECT class FROM Classes ORDER BY '.$sorting_field) as $row) {
        $dens[] = $row['class'];
      }
      $result['dens'] = $dens;

      if ($schema_version == expected_schema_version()) {
        $result['status'] = "ok";
        $result['button'] = 're-initialize';
        $result['message'] = "<p>Schema version is S1".pad3($schema_version).", as expected.</p>\n";
      } else {
        $result['status'] = "unknown";
        $result['button'] = 'update';
        $result['message'] = "<p>Schema version is S1".pad3($schema_version)
            .", but version S1".pad3(expected_schema_version())." would be preferred.</p>\n";
      }
    } catch (PDOException $p) {
      $result['status'] = "trouble";
      $result['button'] = 'initialize';
      $result['message'] = "<p>The database needs to be initialized.</p>\n";
    }

    try {
      $db->exec('DELETE FROM RaceInfo WHERE itemkey = \'NoSuchKey\'');
      $result['writable'] = true;
    } catch (PDOException $p) {
    }
    
    $racer_photo_directory = "";
    $car_photo_directory = "";
    try {
      $racer_photo_directory = read_raceinfo('photo-directory');
      $car_photo_directory = read_raceinfo('car-photo-directory');
      $result['settings_details'] = "Racer photos: ".$racer_photo_directory."<br/>\n"
          ."Car photos: ".$car_photo_directory;
    } catch (PDOException $p) {
    }

    if (!is_dir($racer_photo_directory) || !is_readable($racer_photo_directory) ||
        !is_dir($car_photo_directory) || !is_readable($car_photo_directory)) {
      $result['settings_icon'] = "trouble";
    } else if (!is_writable($racer_photo_directory) || !is_writable($car_photo_directory)) {
      $result['settings_icon'] = "readonly";
    } else if (get_lane_count() == 0) {
      $result['settings_icon'] = 'unknown';
      $result['settings_details'] .= '<br/>The number of lanes on the track hasn\'t been set.';
    } else {
      $result['settings_icon'] = "ok";
      $result['settings_details'] .= '<br/>The track has '.get_lane_count().' lanes.';
    }
  }
  
  return $result;
}


// Builds an array of data for populating the (advanced) database setup dialog.
function build_form_fields($db_connection_string) {
  // Default values, in case $db_connection string is unparseable:
  $form_fields = array('drivers' => pdo_drivers(),
                       'radio' => 'sqlite',
                       'mysql_host' => 'localhost',
                       'sqlite_path' => default_file_path().DIRECTORY_SEPARATOR.date('Ymd-Hi').'.sqlite');
  $form_fields['connection_string'] = 'sqlite:'.$form_fields['sqlite_path'];

  if (isset($db_connection_string) && $db_connection_string) {
    $form_fields['connection_string'] = $db_connection_string;
    $parsed = parse_connection_string($db_connection_string);
    if ($parsed['scheme'] == 'sqlite') {
      $form_fields['sqlite_path'] = $parsed['sqlite_path'];
    } else if ($parsed['scheme'] == 'mysql') {
      $form_fields['radio'] = 'mysql';
      $form_fields['mysql_host'] = $parsed['mysql_host'];
      $form_fields['mysql_dbname'] = $parsed['mysql_dbname'];
    } else if ($parsed['scheme'] == 'odbc') {
      $form_fields['radio'] = 'odbc';
      $form_fields['odbc_dsn_name'] = $parsed['odbc_dsn_name'];
    }
  }

  return $form_fields;
}

function plural($n, $single, $plural) {
  if ($n == 0) {
    return "are no ".$plural;
  } else if ($n == 1) {
    return "is one ".$single;
  } else {
    return "are ".$n." ".$plural;
  }
}

// Constructs an array from which the setup page updates its status.  $schema
// array is the result of inspect_database(), above.
function build_setup_details($db_connection_string, $schema) {
  global $db;
  $db_ok = isset($db_connection_string) && isset($db);
  $db_message = connection_string_to_html(isset($db_connection_string)
                                          ? $db_connection_string : '');
  if ($db_ok) {
    if ($schema['status'] != 'ok') {
      // $schema['message'] is already a <p> element
      $db_message = "<p>".$db_message."</p>\n".$schema['message'];
    } else if (!$schema['writable']) {
      $db_message = "<p>".$db_message."<br/><span class='trouble'>The database is read-only.</span></p>\n";
    } else {
      $db_message = "<p>".$db_message."</p>\n";
    }
  } else {
    $db_message = $schema['message'];
  }
  return array('database' => array('status' =>  $db_ok ? 'ok' : 'trouble',
                                   'writable' => $schema['writable'],
                                   'details' => $db_message),
               'schema' => array('status' => $schema['status'],
                                 'button' => $schema['button'],
                                 'details' => $schema['message']),
               'roster' => array('status' => $schema['nracers'] == 0 ? "unknown" : "ok",
                                 'details' =>
                                 "<p>There ".plural($schema['nracers'], "racer", "racers")." registered."
                                 ."  (Visit the <a href=\"checkin.php\">Check-In page</a>"
                                 ." to enter individual racers.)</p>\n"),
               'classes' => array('status' => count($schema['dens']) == 0 ? "unknown" : "ok",
                                  'label' => $schema['group_label'],
                                  'details' =>
                                  count($schema['dens']) == 0
                                  ? "<p>There are no ".$schema['group_label_lc']."s defined.</p>\n"
                                  : "<p>".implode(", ", $schema['dens'])."</p>\n"),
               'awards' => array('status' => $schema['nawards'] == 0 ? "unknown" : "ok",
                                 'details' => "<p>There ".plural($schema['nawards'], "award", "awards")." defined."
                                 ."  (Visit the <a href=\"awards-editor.php\">Awards Editor page</a>"
                                 ." to enter individual awards.)</p>\n"),
               'settings' => array('status' => $schema['settings_icon'],
                                   'details' => "<p>".$schema['settings_details']."</p>\n"),
               'form_fields' => build_form_fields($db_connection_string)
    );
} 

?>