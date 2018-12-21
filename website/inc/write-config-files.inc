<?php

function local_file_name($filename) {
  $configdir = isset($_SERVER['CONFIG_DIR']) ? $_SERVER['CONFIG_DIR'] : 'local';
  return $configdir.DIRECTORY_SEPARATOR.$filename;
}

function write_config_file($filename, $content) {
  global $g_errstr;
  $where = local_file_name($filename);
  if (@file_put_contents($where, $content, FILE_USE_INCLUDE_PATH) === false) {
    echo "<failure code='cant_write_config'>Can't write config file $where: $g_errstr</failure>";
    return false;
  }

  return true;
}

// Returns true if successful
// ====================================================================
function write_database_config_file($connection_string, $dbuser, $dbpass, $options) {
  // The MYSQL_ATTR_INIT_COMMAND command is, of course, MySQL-specific, but
  // other databases should just ignore it.
  //
  // The setAttribute for ATTR_CASE allows interoperation with GPRM SQLite
  // databases.
  $exported_options = var_export($options, /* return */ true);
  $exported_connection = var_export($connection_string, true);
  $exported_dbuser = var_export($dbuser, true);
  $exported_dbpass = var_export($dbpass, true);
  $content = <<<END
<?php
\$db_connection_string = $exported_connection;
\$db = new PDO(\$db_connection_string, $exported_dbuser, $exported_dbpass,
               $exported_options);

\$db->setAttribute(PDO::ATTR_CASE, PDO::CASE_LOWER);
?>
END;
  return write_config_file('config-database.inc', $content);
}
// ====================================================================

// Returns true if successful
// ====================================================================
function write_config_roles_file($filename) {
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
                                    | ASSIGN_RACER_IMAGE_PERMISSION | JUDGING_PERMISSION | PHOTO_UPLOAD_PERMISSION
                                    | EDIT_RACER_PERMISSION | REGISTER_NEW_RACER_PERMISSION),
                'RaceCoordinator' => array('password' => 'doyourbest',
                                           'permissions' => -1)
	       );
\$post_setup_role = 'RaceCoordinator';
?>

END;
    return write_config_file($filename, $content);
}
// ====================================================================

?>