<?php

require_once('inc/parse-connection-string.inc');
require_once('inc/default-database-directory.inc');
require_once('inc/locked.inc');
require_once('inc/plural.inc');

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
      }
    } catch (PDOException $p) {
    }
  }

  return false;
}

function parse_ini_size($size) {
  $size = trim($size);
  $unit = stripos('KMG', substr($size, -1));
  if ($unit < 0) {
    return round($size);
  }
  return round(substr($size, 0, -1) * pow(1024, 1 + $unit));
}

function connection_string_to_html($conn_string) {
  $parsed = parse_connection_string($conn_string);
  if ($parsed['scheme'] == 'sqlite') {
    return "SQLITE: ".htmlspecialchars($parsed['sqlite_path'], ENT_QUOTES, 'UTF-8');
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
  $result = array('schema_status' => 'trouble',  // icon for schema step
                  // What the button for schema step should say.  The special value 'disabled'
                  // means the button should be disabled (because the database isn't defined).
                  'schema_button' => 'disabled',
                  'schema_message' => "<p><span class='trouble'>Schema not available without a database; "
                                      ."configure the database first.</span></p>",
                  'group_label' => 'Den',
                  'group_label_lc' => 'den',
                  'nracers' => 0,
                  'dens' => array(),
                  'div_rule' => 'by-partition',
                  'nawards' => 0,
                  'nheats' => 0,
                  'nresults' => 0,
                  'fake-racers' => 0,
                  'settings_icon' => 'unknown',
                  'settings_details' => 'No photo directories defined.',
                  'writable' => false
    );

  if (!isset($db) || !$db) {
  } else {
    try {
      $schema_version = schema_version();
      $result['group_label'] = read_raceinfo('partition-label', 'Den');
      $result['group_label_lc'] = strtolower($result['group_label_lc']);
      
      $result['nracers'] = read_single_value('SELECT COUNT(*) FROM RegistrationInfo', array());
      $result['nawards'] = read_single_value('SELECT COUNT(*) FROM Awards', array());
      $result['nheats'] = read_single_value('SELECT COUNT(DISTINCT heat) FROM RaceChart');
      $result['nresults'] = read_single_value('SELECT COUNT(DISTINCT heat) FROM RaceChart'
                                              .' WHERE finishtime IS NOT NULL'
                                              .'  OR finishplace IS NOT NULL');
      $result['fake-racers'] = read_raceinfo('fake-racers', 0);

      $sorting_field = 'sortorder';
      if ($schema_version < 2) { $sorting_field = 'class'; }

      $dens = array();
      foreach ($db->query('SELECT class FROM Classes'
                          .($schema_version < 3 ? '' : ' WHERE constituents IS NULL or constituents = \'\'')
                          .' ORDER BY '.$sorting_field) as $row) {
        $dens[] = $row['class'];
      }
      $result['dens'] = $dens;
      $result['div_rule'] = read_raceinfo('group-formation-rule', 'by-partition');

      if ($schema_version == expected_schema_version()) {
        $result['schema_status'] = "ok";
        $result['schema_button'] = 're-initialize';
        $result['schema_message'] =
            "<p>Schema version is S1".pad3($schema_version).", as expected."
            .(!locked_settings() ? "<br/>"
              ." You can <a href='import-snapshot.php'>Import a database</a>"
              ." if you previously captured one."
              : "")
            ."</p>\n";
      } else {
        $result['schema_status'] = "unknown";
        $result['schema_button'] = 'update';
        $result['schema_message'] = "<p>Schema version is S1".pad3($schema_version)
            .", but <b>version S1".pad3(expected_schema_version())." would be preferred.</b></p>\n";
      }
    } catch (PDOException $p) {
      $result['schema_status'] = "trouble";
      $result['schema_button'] = 'initialize';
      $result['schema_message'] = "<p>The database needs to be initialized.</p>\n";
    }

    try {
      $db->exec('DELETE FROM RaceInfo WHERE itemkey = \'NoSuchKey\'');
      $result['writable'] = true;
    } catch (PDOException $p) {
    }
    
    $racer_photo_directory = "";
    $car_photo_directory = "";
    if (locked_settings()) {
      $result['settings_details'] = "";
      if (get_lane_count() == 0) {
        $result['settings_icon'] = 'unknown';
        $result['settings_details'] .= '<br/><span class=\'trouble\'>The number of lanes on the track hasn\'t been set.</span>';
      } else {
        $result['settings_icon'] = "ok";
        $result['settings_details'] .= '<br/>The track has '.get_lane_count().' lanes.';
      }
    } else {
      try {
        $racer_photo_directory = read_raceinfo('photo-directory');
        $car_photo_directory = read_raceinfo('car-photo-directory');
        $result['settings_details'] = "Racer photos: ".$racer_photo_directory."<br/>\n"
        ."Car photos: ".$car_photo_directory;
      } catch (PDOException $p) {
      }
      if (parse_ini_size(ini_get('upload_max_filesize')) < 8000 * 1024) {
        $result['settings_details'] .= '<br/><span class=\'trouble\'>'
        .'Check your php.ini file if you plan to upload photos: upload_max_filesize='.ini_get('upload_max_filesize')
        .'</span>';
      }

      if (!is_dir($racer_photo_directory) || !is_readable($racer_photo_directory) ||
          !is_dir($car_photo_directory) || !is_readable($car_photo_directory)) {
        $result['settings_icon'] = "trouble";
      } else if (!is_writable($racer_photo_directory) || !is_writable($car_photo_directory)) {
        $result['settings_icon'] = "readonly";
      } else if (get_lane_count() == 0) {
        $result['settings_icon'] = 'unknown';
        $result['settings_details'] .= '<br/><span class=\'trouble\'>The number of lanes on the track hasn\'t been set.</span>';
      } else {
        $result['settings_icon'] = "ok";
        $result['settings_details'] .= '<br/>The track has '.get_lane_count().' lanes.';
      }
    }
  }
  return $result;
}


// Builds an array of data for populating the (advanced) database setup dialog.
function build_form_fields_for_advanced_database_dialog($db_connection_string) {
  // Default values, in case $db_connection string is unparseable:
  $form_fields = array('drivers' => pdo_drivers(),
                       'radio' => 'sqlite',
                       'sqlite_path' => default_database_directory().DIRECTORY_SEPARATOR.date('Ymd-Hi').'.sqlite');
  $form_fields['connection_string'] = 'sqlite:'.$form_fields['sqlite_path'];

  if (isset($db_connection_string) && $db_connection_string) {
    $form_fields['connection_string'] = $db_connection_string;
    $parsed = parse_connection_string($db_connection_string);
    if ($parsed['scheme'] == 'sqlite') {
      $form_fields['sqlite_path'] = $parsed['sqlite_path'];
    } else if ($parsed['scheme'] == 'odbc') {
      $form_fields['radio'] = 'odbc';
      $form_fields['odbc_dsn_name'] = $parsed['odbc_dsn_name'];
    }
  }

  return $form_fields;
}

function plural_to_be($n, $single, $plural) {
  if ($n == 0) {
    return "are no ".$plural;
  } else if ($n == 1) {
    return "is one ".$single;
  } else {
    return "are ".$n." ".$plural;
  }
}


// If we haven't got a working database, look into why.
function diagnose_configuration() {
  global $db_connection_string;

  $config_dir = isset($_SERVER['DERBYNET_CONFIG_DIR'])
      ? $_SERVER['DERBYNET_CONFIG_DIR']
      // dirname 2 levels up, because we're in the /inc subdirectory
      : (dirname(dirname(__FILE__)).DIRECTORY_SEPARATOR.'local');

  $config_file = $config_dir.DIRECTORY_SEPARATOR."config-database.inc";

  if (isset($db_connection_string)) {
    $message = "<p><span class='trouble'>There was a problem opening the database.</span></p>\n";
  } else if (file_exists($config_file)) {
    $message = "<p><span class='trouble'>The configuration file could not be read</span></p>\n";
  } else if (!is_dir($config_dir)) {
    $message = "<p><span class='trouble'>You need to create a <b>'".$config_dir."'</b> "
            ."directory, and make it writable.<span></p>\n";
  } else if (!is_writable($config_dir)) {
    $message = "<p><span class='trouble'>The <b>'".$config_dir."'</b> directory exists, but isn't writable.</span></p>\n";
  } else {
    $message = "<p><span class='trouble'>Schema not available without a database; "
                 ."configure the database first.</span></p>";
  }

  return array('db_message' => $message,
               'schema_status' => 'unknown',  // icon for schema step
               // What the button for schema step should say.  The special value 'disabled'
               // means the button should be disabled (because the database isn't defined).
               'schema_button' => 'disabled',
               'schema_message' => "<p>The database schema isn't available without a database; "
               ."configure the database first.</p>",
               'group_label' => 'Den',
               'group_label_lc' => 'den',
               'nracers' => 0,
               'dens' => array(),
               'div_rule' => 'by-partition',
               'nawards' => 0,
               'nheats' => 0,
               'nresults' => 0,
               'fake-racers' => 0,
               'settings_icon' => 'unknown',
               'settings_details' => 'No photo directories defined.',
               'writable' => false
    );
}

// Constructs an array from which the setup page updates its status.  Returns an
// array with one entry per "step" on the set-up page; each entry is an array of
// details meaningful to that step's display.
function build_setup_page_details() {
  global $db_connection_string;
  global $db;

  $db_message = connection_string_to_html(isset($db_connection_string)
                                          ? $db_connection_string : '');
  $db_ok = isset($db_connection_string) && isset($db);
  if ($db_ok) {
    $schema = inspect_database();
    if ($schema['schema_status'] == 'ok' && !$schema['writable']) {
      $db_message = "<p>".$db_message."<br/><span class='trouble'>The database is read-only.</span></p>\n";
    } else {
      $db_message = "<p>".$db_message."</p>\n";
    }
  } else {
    $schema = diagnose_configuration();
    $db_message = $schema['db_message'];
  }
  return array('locked' => locked_settings(),
               'database' => array('status' =>  $db_ok ? 'ok' : 'trouble',
                                   'writable' => $schema['writable'],
                                   'details' => $db_message),
               'schema' => array('status' => $schema['schema_status'],
                                 'button' => $schema['schema_button'],
                                 'details' => $schema['schema_message']),
               'purge' => array('nracers' => $schema['nracers'],
                                'nawards' => $schema['nawards'],
                                'nheats' => $schema['nheats'],
                                'nresults' => $schema['nresults']),
               'roster' => array('status' => $schema['nracers'] == 0 ? "unknown" : "ok",
                                 'count' => $schema['nracers'],
                                 'fake' => $schema['fake-racers'],
                                 'details' =>
                                 "<p>There ".plural_to_be($schema['nracers'], "racer", "racers")." registered."
                                 ."  (Visit the <a href=\"checkin.php\">Check-In page</a>"
                                 ." to enter individual racers.)</p>\n"),
               'groups' => array('status' => count($schema['dens']) == 0 ? "unknown" : "ok",
                                 'label' => $schema['group_label'],
                                 'plural' => plural($schema['group_label']),
                                 'details' =>
                                 "<p>".
                                 ($schema['div_rule'] == 'by-partition'
                                  ? "Racing by ".$schema['group_label_lc'].": "
                                  : ($schema['div_rule'] == 'one-group'
                                     ? "Racing as one big group: "
                                     : "Customized groups: ")).
                                 (count($schema['dens']) == 0
                                  ? "There are no ".plural($schema['group_label_lc'])." defined."
                                  : implode(", ", $schema['dens']))
                                 ."</p>\n"),
               'awards' => array('status' => $schema['nawards'] == 0 ? "unknown" : "ok",
                                 'details' => "<p>There ".plural_to_be($schema['nawards'], "award", "awards")." defined."
                                 ."  (Visit the <a href=\"awards-editor.php\">Awards Editor page</a>"
                                 ." to enter individual awards.)</p>\n"),
               'settings' => array('status' => $schema['settings_icon'],
                                   'details' => "<p>".$schema['settings_details']."</p>\n"),
               'form_fields' => build_form_fields_for_advanced_database_dialog($db_connection_string)
    );
}
?>
