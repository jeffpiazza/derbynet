<?php session_start();

require_once('inc/authorize.inc');
require_once('inc/banner.inc');
require_permission(SET_UP_PERMISSION);

?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>DerbyNet Set-Up</title>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery-ui-1.10.4.min.js"></script>
<link rel="stylesheet" type="text/css" href="css/setup.css"/>
<script type="text/javascript" src="js/mobile-init.js"></script>
<script type="text/javascript" src="js/jquery.mobile-1.4.2.min.js"></script>
<script type="text/javascript" src="js/dashboard-ajax.js"></script>
<script type="text/javascript" src="js/modal.js"></script>
<script type="text/javascript" src="js/chooser.js"></script>
<script type="text/javascript" src="js/setup.js"></script>
</head>
<body>
<?php make_banner('Set-Up'); ?>
<?php

require_once('inc/ajax-failure.inc'); // Must follow jquery
require_once('inc/parse_connection_string.inc');
require_once('inc/default-file-path.inc');
$default_file_path = default_file_path();

$configdir = isset($_SERVER['CONFIG_DIR']) ? $_SERVER['CONFIG_DIR'] : 'local';
try {
  @include($configdir.DIRECTORY_SEPARATOR."config-database.inc");
} catch (PDOException $p) {
}

if (isset($db) && $db) {
  require_once('inc/data.inc');
  require_once('inc/schema_version.inc');
}

// TODO Work out the "E-Z Setup" strategy.  Where does the baseline directory get defined?
// TODO <select> control to show existing E-Z-Setup databases.  Don't show if there are no E-Z databases.

// If there's an existing database config, then $db_connection_string should
// contain the connection string (having been set in data.inc).  We parse the
// connection string to figure out how to populate the fields of the "advanced"
// database dialog.

$schema = inspect_database();

$initial_details = build_setup_details(@$db_connection_string, $schema);
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
    <input type="button" data-enhanced="true" value="Database" onclick="show_database_modal()"/>
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
    <form method="link" action="import-roster.php">
      <input type="submit" data-enhanced="true" value="Import Roster"/>
    </form>
  </div>

  <div class="step_details"></div>
</div>

<!-- Classes -->
<div id="classes_step" class="step_div">
  <div class="status_icon"><img/></div>

  <div class="step_button block_buttons">
    <form method="link" action="class-editor.php">
      <input type="submit" data-enhanced="true"/>
    </form>
  </div>

  <div class="step_details"></div>
</div>

<!-- Awards -->
<div id="awards_step" class="step_div">
  <div class="status_icon"><img/></div>

  <div class="step_button block_buttons">
    <form method="link" action="import-awards.php">
      <input type="submit" data-enhanced="true" value="Import Awards"/>
    </form>
  </div>

  <div class="step_details"></div>
</div>

<!-- Photo directories -->
<div id="photo_step" class="step_div">
  <div class="status_icon"><img/></div>

  <div class="step_button block_buttons">
    <form method="link" action="import-awards.php">
      <input type="submit" data-enhanced="true" value="Settings"/>
    </form>
  </div>

  <div class="step_details"></div>
</div>

<!-- TODO Settings?  Just show the non-standard ones? -->

<div id="database_modal" class="modal_dialog hidden block_buttons">
  <form>
    <div id="ez_database">
      <label for="ez_database_name">Database name:</label>
      <input type="text" name="ez_database_name" id="ez_database_name"/>
    </div>

    <input type="submit" data-enhanced="true" value="Submit"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick="close_modal('#database_modal');"/>
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

    <div id="for_string_connection">
        <label for="connection_string">Database connection string:</label>
        <input type="text" name="connection_string" id="connection_string"
            <?php
              if (!isset($db_connection_string)) {
                echo 'placeholder="Database connection string"';
              }
            ?>/>
    </div>

    <label for="dbuser">Database user name:</label>
    <input type="text" name="dbuser" id="username_field"/>
    <label for="dbpass">Database password:</label>
    <input type="text" name="dbpass" id="password_field"/>

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

<?php require('inc/chooser.inc'); ?>
  
</body>
</html>
