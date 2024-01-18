<?php

function data_folder_file_path($filename) {
  $configdir = isset($_SERVER['DERBYNET_CONFIG_DIR']) ? $_SERVER['DERBYNET_CONFIG_DIR'] : 'local';
  return $configdir.DIRECTORY_SEPARATOR.$filename;
}

function write_config_file($filename, $content) {
  global $g_errstr;
  $where = data_folder_file_path($filename);
  if (@file_put_contents($where, $content, FILE_USE_INCLUDE_PATH) === false) {
    json_failure('cant_write_config', "Can't write config file $where: $g_errstr");
    return false;
  }

  if (function_exists('opcache_invalidate')) {
    opcache_invalidate($where);
  }
  return true;
}

// Returns true if successful
// ====================================================================
function write_database_config_file($connection_string, $homedir, $dbuser, $dbpass, $options) {
  // The setAttribute for ATTR_CASE allows interoperation with GPRM SQLite
  // databases.
  $exported_options = var_export($options, /* return */ true);
  $exported_connection = var_export($connection_string, true);
  $exported_dbuser = var_export($dbuser, true);
  $exported_dbpass = var_export($dbpass, true);
  $exported_homedir = var_export($homedir, true);
  $content = <<<END
<?php
\$homedir = $exported_homedir;
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
