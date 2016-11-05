<?php
@session_start();

// Formerly action.setup.inc, but this can't be an ajax action run
// through action.php, because inc/data.inc will force a redirect if
// there's no configured database.

// $_POST['connection_string'], e.g., 'mysql:host=localhost;dbname=trial3'
// $_POST['dbuser']
// $_POST['dbpass']

require_once('inc/permissions.inc');
require_once('inc/authorize.inc');

require_once('inc/action-helpers.inc');


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


header('Content-Type: text/xml; charset=utf-8');
echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
start_response();

if (have_permission(SET_UP_PERMISSION)) {
  $ok = true;

  $connection_string = $_POST['connection_string'];
  $dbuser = $_POST['dbuser'];
  $dbpass = $_POST['dbpass'];

  $options = array();
  if (defined('PDO::MYSQL_ATTR_INIT_COMMAND')) {
    $options[PDO::MYSQL_ATTR_INIT_COMMAND] = "SET NAMES utf8";
  }

  try {
    $trial_db = new PDO($connection_string, $dbuser, $dbpass, $options);
  } catch (PDOException $p) {
    $ok = false;
    echo "<failure code='cant_connect'>";
    var_dump($p);
    echo "</failure>\n";
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
\$db = new PDO($connection_exported, $dbuser_exported, $dbpass_exported,
               $options_exported);
?>
END;
  }

  if ($ok) {
    $ok = write_config_file('config-database.inc', $content);
  }

  $config_roles = 'config-roles.inc';
  if ($ok && !file_exists(local_file_name($config_roles))) {
    $content = <<<END
<?php
\$roles = array('' => 
	       array('password' => '',
		     'permissions' =>
		     VIEW_RACE_RESULTS_PERMISSION),
	       'RaceCrew' =>
	       array('password' => 'murphy',
		     'permissions' =>
		     VIEW_RACE_RESULTS_PERMISSION | VIEW_AWARDS_PERMISSION
		     | CHECK_IN_RACERS_PERMISSION | REVERT_CHECK_IN_PERMISSION
             | ASSIGN_RACER_IMAGE_PERMISSION
             | EDIT_RACER_PERMISSION | REGISTER_NEW_RACER_PERMISSION),
	       'RaceCoordinator' =>
	       array('password' => 'doyourbest',
		     'permissions' => -1)
	       );
\$post_setup_role = 'RaceCoordinator';
?>

END;
    $ok = write_config_file($config_roles, $content);
    }

  if ($ok) {
    echo "<success/>\n";
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
