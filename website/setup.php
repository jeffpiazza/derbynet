<?php session_start();

require_once('inc/authorize.inc');
require_once('inc/banner.inc');
require_permission(SET_UP_PERMISSION);

?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>DerbyNet Set-Up</title>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/setup.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery-ui-1.10.4.min.js"></script>
<script type="text/javascript" src="js/mobile-init.js"></script>
<script type="text/javascript" src="js/jquery.mobile-1.4.2.min.js"></script>
<script type="text/javascript" src="js/modal.js"></script>
<script type="text/javascript" src="js/chooser.js"></script>
<script type="text/javascript" src="js/setup.js"></script>
</head>
<body>
<?php make_banner('Set-Up'); ?>
<?php

require_once('inc/ajax-failure.inc'); // Must follow jquery
require_once('inc/parse-connection-string.inc');
require_once('inc/details-for-setup-page.inc');
require_once('inc/default-database-directory.inc');
require_once('inc/standard-configs.inc');
require_once('inc/locked.inc');


$configdir = isset($_SERVER['CONFIG_DIR']) ? $_SERVER['CONFIG_DIR'] : 'local';
try {
  @include($configdir.DIRECTORY_SEPARATOR."config-database.inc");
} catch (PDOException $p) {
}

if (isset($db) && $db) {
  require_once('inc/data.inc');
  require_once('inc/schema_version.inc');
}


// If there's an existing database config, then $db_connection_string should
// contain the connection string (having been set in data.inc).  We parse the
// connection string to figure out how to populate the fields of the "advanced"
// database dialog.

$initial_details = build_setup_page_details();

$ez_configs = list_standard_configs(default_database_directory());
?>
<script type="text/javascript">
//<![CDATA[
$(function() { populate_details(<?php echo json_encode($initial_details); ?>); });
//]]>
</script>

<!-- Database -->
<div id="database_step" class="step_div">
  <div class="status_icon"><img/></div>

  <div class="step_button block_buttons">
    <input type="button" data-enhanced="true" value="Choose Database" onclick="show_ezsetup_modal()"/>
  </div>

  <div class="step_details"></div>
</div>

<!-- Schema -->
<div id="schema_step" class="step_div">
  <div class="status_icon"><img/></div>

  <div class="step_button block_buttons">
    <input type="button" data-enhanced="true"/>
  </div>

  <div class="step_details"></div>
</div>

<!-- Roster -->
<div id="roster_step" class="step_div">
  <div class="status_icon"><img/></div>

  <div class="step_button block_buttons">
    <a class="button_link" href="import-roster.php">Import Roster</a>
  </div>

  <div class="step_details"></div>
</div>

<!-- Classes -->
<div id="classes_step" class="step_div">
  <div class="status_icon"><img/></div>

  <div class="step_button block_buttons">
    <a class="button_link" href="class-editor.php"></a>
  </div>

  <div class="step_details"></div>
</div>

<!-- Awards -->
<div id="awards_step" class="step_div">
  <div class="status_icon"><img/></div>

  <div class="step_button block_buttons">
    <a class="button_link" href="import-awards.php">Import Awards</a>
  </div>

  <div class="step_details"></div>
</div>

<!-- Photo directories and lane count -->
<div id="settings_step" class="step_div">
  <div class="status_icon"><img/></div>

  <div class="step_button block_buttons">
    <a class="button_link" href="settings.php">Settings</a>
  </div>

  <div class="step_details"></div>
</div>


<div id="ezsetup_modal" class="modal_dialog hidden block_buttons">
  <form>
    <input type="hidden" name="action" value="setup.nodata"/>

    <div id="ez_database">
      <label for="ez_database_name">Name for new database:</label>
      <input type="text" name="ez-new" id="ez_database_name"/>
    </div>

<?php
if (count($ez_configs) > 0) {
?>
<p style="text-align: center;"><b>OR</b></p>
    <select name="ez-old" id="ez_database_select">
<option id="ez-old-nochoice" disabled="disabled" selected="selected" value="">Please select:</option>
<?php
    foreach ($ez_configs as $config) {
      $path = htmlspecialchars($config['relpath'], ENT_QUOTES, 'UTF-8');
      echo "<option value='".$path."'>".$path."</option>\n";
} ?>    </select>
<?php } ?>

    <br/>
    <input type="submit" data-enhanced="true" value="Submit"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick="close_modal('#ezsetup_modal');"/>
    <br/>
    <input type="button" data-enhanced="true" value="Advanced"
      onclick="show_advanced_database_modal()"/>
  </form>
</div>


<div id="advanced_database_modal" class="modal_dialog wide_modal tall_modal hidden block_buttons">
  <form><?php
    $is_windows = strtoupper(substr(PHP_OS, 0, 3)) === 'WIN';
?>
    <input type="hidden" name="action" value="setup.nodata"/>

    <input id="sqlite_connection" type="radio" name="connection_type" value="sqlite"/>
    <label for="sqlite_connection">SQLite data source<span class="missing_driver"></span></label>

    <input id="mysql_connection" type="radio" name="connection_type" value="mysql"/>
    <label for="mysql_connection">MySQL data source<span class="missing_driver"></span></label>

    <input id="odbc_connection" type="radio" name="connection_type" value="odbc"/>
    <label for="odbc_connection">ODBC data source<span class="missing_driver"></span></label>

    <input type="radio" name="connection_type" value="string" id="string_connection"/>
    <label for="string_connection">Arbitrary connection string</label>

    <div id="for_odbc_connection" class="hidden connection_details">
        <label for="odbc_dsn_name">ODBC data source name (DSN):</label>
        <input type="text" name="odbc_dsn_name" id="odbc_dsn_name"/>
    </div>

    <div id="for_mysql_connection" class="hidden connection_details">
        <label for="mysql_host">MySQL Host:</label>
        <input type="text" name="mysql_host" id="mysql_host"/>
        <label for="mysql_dbname">MySQL database name:</label>
        <input type="text" name="mysql_dbname" id="mysql_dbname"/>
    </div>

    <div id="for_sqlite_connection" class="hidden connection_details">
      <input type="button" data-enhanced="true" id="browse_for_sqlite"
       value="Browse"
       onclick='show_choose_file_modal($("#sqlite_path").val(), "new.sqlite",
                                       function(fullpath) {
                                         $("#sqlite_path").val(fullpath);
                                         update_sqlite_path();
                                       });'/>
      <div style="width: 80%">
        <label for="sqlite_path">Path (on server) to SQLite data file:</label>
        <input type="text" name="sqlite_path" id="sqlite_path"/>
      </div>
    </div>

    <label for="dbuser">Database user name:</label>
    <input type="text" name="dbuser" id="username_field"/>
    <label for="dbpass">Database password:</label>
    <input type="text" name="dbpass" id="password_field"/>

    <div id="for_string_connection">
        <label for="connection_string">Database connection string:</label>
        <input type="text" name="connection_string" id="connection_string"
            <?php
              if (!isset($db_connection_string)) {
                echo 'placeholder="Database connection string"';
              }
            ?>/>
    </div>

    <input type="submit" data-enhanced="true" value="Submit"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick='close_modal("#advanced_database_modal");'/>
  </form>
</div>

<div id="initialize_schema_modal" class="modal_dialog hidden block_buttons">
  <form>

    <p>Initializing the schema will wipe out any existing data in the 
    database, and cannot be undone.  Are you sure that's what you want to do?</p>

    <input type="submit" data-enhanced="true" value="Initialize"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick='close_modal("#initialize_schema_modal");'/>
  </form>
</div>

<div id="update_schema_modal" class="modal_dialog hidden block_buttons"> <form>

  <p>Before updating the schema, it's wise to back up any important data
      that's already in the database.  Updating the schema cannot be undone.
      Are you sure that's what you want to do?</p>

    <input type="submit" data-enhanced="true" value="Update Schema"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick='close_modal("#update_schema_modal");'/>
  </form>
</div>

<div id="reporting_box" class="hidden">
  <div id="reporting_box_content"></div>
  <div id="reporting_box_dismiss" class="hidden"
       onclick="hide_reporting_box(); return false;">Dismiss</div>
</div>

<?php require('inc/chooser.inc'); ?>
  
</body>
</html>
