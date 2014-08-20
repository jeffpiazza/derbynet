<?php

// $_POST['connection_string'], e.g., 'mysql:host=localhost;dbname=trial3'

// $_POST['dbuser']
// $_POST['dbpass']

function local_file_name($filename) {
  return 'local/'.$filename;
}


function error_handler($errno, $errstr, $errfile, $errline)
{
  $g_errstr = $errstr;
  return false;  // allow normal handling to continue
}

set_error_handler('error_handler');

function write_config_file($filename, $content) {
  if (@file_put_contents(local_file_name($filename), $content, FILE_USE_INCLUDE_PATH) === false) {
    echo "<failure code='cant_write_config'>Can't write config file: $g_errstr</failure>";
    return false;
  }

  return true;
}


start_response();

if (have_permission(SET_UP_PERMISSION)) {
  $ok = true;

  $connection_string = $_POST['connection_string'];
  $dbuser = $_POST['dbuser'];
  $dbpass = $_POST['dbpass'];

  try {
    $trial_db = new PDO($connection_string, $dbuser, $dbpass,
                        array(PDO::MYSQL_ATTR_INIT_COMMAND => "SET NAMES utf8"));
  } catch (PDOException $p) {
    $ok = false;
    echo "<failure code='cant_connect'>";
    var_dump($p);
    echo "</failure>\n";
  }


  if ($ok) {
    // The MYSQL command is, of course, MySQL-specific, but ODBC/Access should just ignore it.
    $content = <<<END
<?php
\$db = new PDO("$connection_string", "$dbuser", "$dbpass",
               array(PDO::MYSQL_ATTR_INIT_COMMAND => "SET NAMES utf8"));
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
             | EDIT_RACER_PERMISSION | REGISTER_NEW_RACER_PERMISSION),
	       'RaceCoordinator' =>
	       array('password' => 'doyourbest',
		     'permissions' => -1)
	       );
?>

END;
    $ok = write_config_file($config_roles, $content);
    }

  if ($ok) {
    echo "<success/>\n";
  }
} else {
	not_authorized_body();
}

end_response();
?>