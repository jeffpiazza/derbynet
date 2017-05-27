<?php
// $_POST['connection_string'], e.g., 'mysql:host=localhost;dbname=trial3'
// $_POST['dbuser']
// $_POST['dbpass']

// TODO A failure encountered while setting up a new database needs to be
// displayed, but doesn't otherwise affect the details on the setup page.  Only
// if successful in setting up the new database (so, set $db), call
// inspect_database and build_setup_details to return.
//
// attempt_to_count_racers may detect additional problems (access denied,
// not_a_database).


require_once('inc/locked.inc');
require_once('inc/parse_connection_string.inc');
require_once('inc/sql-script.inc');

function local_file_name($filename) {
  $configdir = isset($_SERVER['CONFIG_DIR']) ? $_SERVER['CONFIG_DIR'] : 'local';
  return $configdir.DIRECTORY_SEPARATOR.$filename;
}


function error_handler($errno, $errstr, $errfile, $errline)
{
  $g_errstr = $errstr;
  return false;  // allow normal handling to continue
}

set_error_handler('error_handler');

function write_config_file($filename, $content) {
  global $g_errstr;
  if (@file_put_contents(local_file_name($filename), $content, FILE_USE_INCLUDE_PATH) === false) {
    echo "<failure code='cant_write_config'>Can't write config file: $g_errstr</failure>";
    return false;
  }

  return true;
}

// Returns {pdo: <PDO-or-null>, exception: <PDOException>, error: <string-or-null>}
// where <string-or-null> can take the values:
// 'cant_connect'
// 'acess_denied'
function attempt_to_create_database($connection_string, $dbuser, $dbpass, $options) {
  $result = array('pdo' => null, 'exception' => null, 'error' => null);
  try {
    $result['pdo'] = new PDO($connection_string, $dbuser, $dbpass, $options);
  } catch (PDOException $p) {
    $result['exception'] = $p;
    if ($p->getCode() == "1045") {
      // MySQL SQLSTATE[HY000] [1045] Access denied for user ''@'localhost' (using password: NO)"
      $result['error'] = 'access_denied';
    } else if ($p->getCode() == 14) {
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

start_response();

if (locked_settings()) {
  echo "<failure code='locked'>Settings are locked and cannot be changed.</failure>\n";
} else if (have_permission(SET_UP_PERMISSION)) {
  $ok = true;

  $connection_string = $_POST['connection_string'];
  $dbuser = $_POST['dbuser'];
  $dbpass = $_POST['dbpass'];

  $options = array();
  if (defined('PDO::MYSQL_ATTR_INIT_COMMAND')) {
    $options[PDO::MYSQL_ATTR_INIT_COMMAND] = "SET NAMES utf8";
  }

  $trial = attempt_to_create_database($connection_string, $dbuser, $dbpass, $options);
  if ($trial['pdo']) {
    $trial_db = $trial['pdo'];
  } else {
    $ok = false;
    echo "<failure code='".$trial['error']."'>";
    if ($trial['error'] == 'access_denied') {
      echo "Access denied:\n".$trial['exception']->getMessage();
    } else {
      var_dump($trial['exception']);
    }
    echo "</failure>\n";
  }

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
        $ok = run_sql_script('schema', true);
      } else if ($count['error'] == 'not_a_database') {
        $parsed = parse_connection_string($connection_string);
        $sqlite_path = $parsed['sqlite_path'];
        echo "<failure code='not_a_database'>".htmlspecialchars($sqlite_path, ENT_QUOTES, 'UTF-8')
            ."\nis not a database.</failure>";
      } else {
        $p = $count['exception'];
        echo "<failure code='unknown_sql'"
             ." sqlcode='".htmlspecialchars($p->getCode(), ENT_QUOTES, 'UTF-8')."'>\n";
        echo "<![CDATA[\n";
        echo "CODE: ".$p->getCode()."\n";
        echo "ERROR: ".$trial_db->errorInfo()[1]."\n";
        var_dump($p);
        echo "\n]]>\n";
        echo "</failure>\n";
      }
    }
  }

  if ($ok) {
    // The MYSQL_ATTR_INIT_COMMAND command is, of course, MySQL-specific, but other databases
    // should just ignore it.
    $options_exported = var_export($options, /* return */ true);
    $connection_exported = var_export($connection_string, true);
    $dbuser_exported = var_export($dbuser, true);
    $dbpass_exported = var_export($dbpass, true);
    $content = <<<END
<?php
\$db_connection_string = $connection_exported;
\$db = new PDO(\$db_connection_string, $dbuser_exported, $dbpass_exported,
               $options_exported);
?>
END;
    $ok = write_config_file('config-database.inc', $content);
  }

  $config_roles = 'config-roles.inc';
  if ($ok && !file_exists(local_file_name($config_roles))) {
    $content = <<<END
<?php
\$roles = array('' => array('password' => '',
                            'permissions' => VIEW_RACE_RESULTS_PERMISSION),
                'Timer' => array('password' => '',
                                 'permissions' => TIMER_MESSAGE_PERMISSION,
                                 'interactive' => false),
                'Photo' => array('password' => 'flashbulb',
                                 'permissions' => PHOTO_UPLOAD_PERMISSION,
                                 'interactive' => false),
                'RaceCrew' => array('password' => 'murphy',
                                    'permissions' =>
                                    VIEW_RACE_RESULTS_PERMISSION | VIEW_AWARDS_PERMISSION
                                    | CHECK_IN_RACERS_PERMISSION | REVERT_CHECK_IN_PERMISSION
                                    | ASSIGN_RACER_IMAGE_PERMISSION | PHOTO_UPLOAD_PERMISSION
                                    | EDIT_RACER_PERMISSION | REGISTER_NEW_RACER_PERMISSION),
                'RaceCoordinator' => array('password' => 'doyourbest',
                                           'permissions' => -1)
	       );
\$post_setup_role = 'RaceCoordinator';
?>

END;
    $ok = write_config_file($config_roles, $content);
  }

  if ($ok) {
    echo "<success/>\n";

    // With the database config file written, we can construct the PDO as most
    // other scripts do.
    require_once('inc/data.inc');

    // Summarize the database state for the user.
    echo "<details><![CDATA[\n";
    echo json_encode(build_setup_details($connection_string, inspect_database()));
    echo "\n]]></details>\n";

    // Setup permissions were granted temporarily because there was no
    // configuration present.  Now that there is, remove the special setting_up
    // flag and log in (without password) as the race coordinator (or whatever
    // other role the config file specifies).
    unset($_SESSION['setting_up']);

    @include_once(local_file_name($config_roles));
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
  not_authorized_body();
}

end_response();
?>
