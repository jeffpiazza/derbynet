<?php
// $_POST['ez-old']
// or
// $_POST['ez-new']
//
// otherwise, Advanced configuration:
// $_POST['connection_string'], e.g., 'mysql:host=localhost;dbname=trial3'
// $_POST['dbuser']
// $_POST['dbpass']


require_once('inc/locked.inc');
require_once('inc/details-for-setup-page.inc');
require_once('inc/sql-script.inc');
require_once('inc/default-database-directory.inc');
require_once('inc/write-config-files.inc');
require_once('inc/standard-configs.inc');

function error_handler($errno, $errstr, $errfile, $errline)
{
  global $g_errstr;
  $g_errstr = $errstr;
  return false;  // allow normal handling to continue
}

set_error_handler('error_handler');

// Returns {pdo: <PDO-or-null>, exception: <PDOException>, error: <string-or-null>}
// where <string-or-null> can take the values:
// 'cant_connect'
// 'acess_denied'
function attempt_to_connect_to_database($connection_string, $dbuser, $dbpass, $options) {
  $result = array('pdo' => null, 'exception' => null, 'error' => null);
  try {
    $result['pdo'] = new PDO($connection_string, $dbuser, $dbpass, $options);
  } catch (PDOException $p) {
    $result['exception'] = $p;
    if ($p->getCode() == 14) {
      // TODO For sqlite, a bad path (non-existent directory)
      //   SQLSTATE[HY000] [14] unable to open database file
      $result['error'] = 'no_such_directory';
    } else {
      $result['error'] = 'cant_connect';
    }
  }
  return $result;
}

// Return {count: <n>, exception: <PDOException>, error: <string-or-null>}
// <string-or-null> can be:
// 'empty'
// 'not_a_database'
// 'unknown'
function attempt_to_count_racers($trial_db) {
  $result = array('count' => null, 'exception' => null, 'error' => null);
  try {
    $trial_db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    $stmt = $trial_db->prepare('SELECT COUNT(*) from RegistrationInfo');
    $stmt->execute(array());
    $row = $stmt->fetch(PDO::FETCH_NUM);
    $stmt->closeCursor();
    $result['count'] = $row[0];
  } catch (PDOException $p) {
    $result['exception'] = $p;
    $error_code = $trial_db->errorInfo()[1];
    if ($p->getCode() == "HY000") {
      // https://www.sqlite.org/c3ref/c_abort.html
      // SQLITE_BUSY         5   /* The database file is locked */
      // SQLITE_LOCKED       6   /* A table in the database is locked */
      // SQLITE_READONLY     8   /* Attempt to write a readonly database */
      // SQLITE_AUTH        23   /* Authorization denied */
      if ($error_code == 1 /* SQLITE_ERROR */) {
        // "Missing database", but this is also returned for
        // General error: 1 no such table: RegistrationInfo
        $result['error'] = 'empty';
      } else if ($error_code == 26 /* SQLITE_NOTADB */) {
        $result['error'] = 'not_a_database';
      } else {
        $result['error'] = 'unknown';
      }
    } else if ($p->getCode() == "42S02") {  // "Base table or view not found"
      $result['error'] = 'empty';
    } else {
      $result['error'] = 'unknown';
    }
  }
  return $result;
}

if (locked_settings()) {
  json_failure('locked', "Settings are locked and cannot be changed.");
} else if (have_permission(SET_UP_PERMISSION)) {
  $ok = true;

  // For a new ez-setup, we'll create subdirectories, and record their paths in
  // these variables.
  $racers_subdir = '';
  $cars_subdir = '';
  $videos_subdir = '';
  $imagery_subdir = '';
  $slides_subdir = '';

  $new_database_path = '';

  if (isset($_POST['ez-new']) && $_POST['ez-new'] != "") {
    $superdir = default_database_directory() . DIRECTORY_SEPARATOR . getdate()['year'];
    $directory = $superdir . DIRECTORY_SEPARATOR . $_POST['ez-new'];

    error_reporting(E_ALL);
    
    if (is_dir($directory)) {
      json_failure('already_exists', "Directory already exists.");
      $ok = false;
    }

    $old_umask = umask(0);

    if ($ok) {
      if (!@mkdir($directory, 0777, true)) {
        $error = error_get_last();
        json_failure('mkdir', $directory.':  '.$error['message']);
        $ok = false;
      }
    }
    // Create subdirectories.  Recording them in the database happens after the
    // database is opened and initialized.
    $racers_subdir = $directory . DIRECTORY_SEPARATOR . 'racers';
    $cars_subdir = $directory . DIRECTORY_SEPARATOR . 'cars';
    $videos_subdir = $directory . DIRECTORY_SEPARATOR . 'videos';
    $logs_subdir = $directory . DIRECTORY_SEPARATOR . 'logs';
    $imagery_subdir = $directory . DIRECTORY_SEPARATOR . 'imagery';
    $slides_subdir = $directory . DIRECTORY_SEPARATOR . 'slides';
    if ($ok) {
      if (!@mkdir($racers_subdir)) {
        json_failure('mkdir-racers', "Unable to create racers subdirectory.");
        $ok = false;
      }
    }
    if ($ok) {
      if (!@mkdir($cars_subdir)) {
        json_failure('mkdir-cars', "Unable to create cars subdirectory.");
        $ok = false;
      }
    }
    if ($ok) {
      if (!@mkdir($videos_subdir)) {
        json_failure('mkdir-videos', "Unable to create videos subdirectory.");
        $ok = false;
      }
    }
    if ($ok) {
      if (!@mkdir($logs_subdir)) {
        json_failure('mkdir-logs', "Unable to create logs subdirectory.");
        $ok = false;
      }
    }
    if ($ok) {
      if (!@mkdir($imagery_subdir)) {
        json_failure('mkdir-imagery', "Unable to create imagery subdirectory.");
        $ok = false;
      }
    }
    if ($ok) {
      if (!@mkdir($slides_subdir)) {
        json_failure('mkdir-slides', "Unable to create slides subdirectory.");
        $ok = false;
      }
    }

    umask($old_umask);

    if ($ok) {
      $new_database_path = $directory . DIRECTORY_SEPARATOR . 'derbynet.sqlite3';
      $connection_string = 'sqlite:'. $new_database_path;
      $homedir = $directory;
      $dbuser = $dbpass = "";
    }
  } else if (isset($_POST['ez-old']) && $_POST['ez-old'] != "") {
    $config = is_standard_config(default_database_directory(), $_POST['ez-old']);
    if ($config === false) {
      json_failure('not-recognized', $_POST['ez-old'].' is not a recognized configuration.');
      $ok = false;
    } else {
      $connection_string = 'sqlite:'.$config['database'];
      $homedir = $config['homedir'];
      $dbuser = $dbpass = "";
    }
  } else {
    $connection_string = $_POST['connection_string'];
    $homedir = false;
    $dbuser = $_POST['dbuser'];
    $dbpass = $_POST['dbpass'];
  }

  if ($ok) {
    $options = array();

    $trial = attempt_to_connect_to_database($connection_string, $dbuser, $dbpass, $options);
    if ($trial['pdo']) {
      $trial_db = $trial['pdo'];
      if ($new_database_path) {
        // By default, a newly-created sqlite database will offer write
        // permissions only to the creator, i.e., the www user.  This can
        // frustrate the actual human user if they ever have occasion to want to
        // access the database directly.
        $old_umask = umask(0);
        chmod($new_database_path, 0777);
        umask($old_umask);
      }
    } else {
      json_failure($trial['error'], $trial['exception']->getMessage());
      $ok = false;
    }
  }

  // Check database integrity; initialize an empty database
  // Needs $trial_db, $connection_string
  if ($ok) {
    $count = attempt_to_count_racers($trial_db);

    if ($count['count'] !== null) {
    } else {
      $ok = false;
      if ($count['error'] == 'empty') {
        // There's no reason not to just initialize an empty database
        $parsed = parse_connection_string($connection_string);
        $db = $trial_db;
        $dbtype = $parsed['scheme'];
        list($ok, $executed) = run_sql_script('schema');
        json_out('executed', $executed);
      } else if ($count['error'] == 'not_a_database') {
        $parsed = parse_connection_string($connection_string);
        $sqlite_path = $parsed['sqlite_path'];
        json_failure('not-a-database', $sqlite_path."\nis not a database.");
      } else {
        $p = $count['exception'];
        json_failure('sql', $p->getMessage());
      }
    }
  }

  if ($ok) {
    $ok = write_database_config_file($connection_string, $homedir, $dbuser, $dbpass, $options);
  }

  $config_roles = 'config-roles.inc';
  if ($ok && !file_exists(data_folder_file_path($config_roles))) {
    write_config_roles_file($config_roles);
  }

  // Configuration should be written to the file system now, so open the
  // database like any other page would.
  if ($ok) {
    require_once('inc/data.inc');

    if ($racers_subdir != '') {
      write_raceinfo('photo-directory', $racers_subdir);
    }
    if ($cars_subdir != '') {
      write_raceinfo('car-photo-directory', $cars_subdir);
    }
    if ($videos_subdir != '') {
      write_raceinfo('video-directory', $videos_subdir);
    }
    if ($logs_subdir != '') {
      write_raceinfo('logs-directory', $logs_subdir);
    }

    json_success();

    // With the database config file written, we can construct the PDO as most
    // other scripts do.

    // Summarize the database state for the user.
    json_out('details', build_setup_page_details());

    // Setup permissions were granted temporarily because there was no
    // configuration present.  Now that there is, remove the special setting_up
    // flag and log in (without password) as the race coordinator (or whatever
    // other role the config file specifies).
    unset($_SESSION['setting_up']);

    @include_once(data_folder_file_path($config_roles));
    if (!isset($post_setup_role)) {
      $post_setup_role = 'RaceCoordinator';
    }
    $_SESSION['role'] = $post_setup_role;
    $role = $roles[$post_setup_role];
    if ($role) {
      $_SESSION['permissions'] = $role['permissions'];
    } else {
      $_SESSION['permissions'] = -1;
    }
  }
} else {
  json_not_authorized();
}
?>
