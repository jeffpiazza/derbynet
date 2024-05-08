<?php @session_start();

require_once('inc/data.inc');
require_once('inc/authorize.inc');
session_write_close();
require_once('inc/banner.inc');

require_permission(SET_UP_PERMISSION);

?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Import Snapshot</title>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/import-snapshot.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/ajax-setup.js"></script>
<script type="text/javascript" src="js/import-snapshot.js"></script>
</head>
<body>
<?php make_banner('Import Database Snapshot', 'setup.php'); ?>

<div class="warning">
    <span class="red">WARNING:</span> Importing a database snapshot<br/>
    will <b>delete</b> all the data<br/>
    that's currently in the database.
</div>

<form id="upload-form">
  <input type="hidden" name="action" value="snapshot.put"/>
  <div class="file_target">
    <input type="file" id="snapshot_file" name="snapshot"/>
    <label for="snapshot_file">
      <div id="drop_img"><img src="img/drop.png"/></div>
      <div id="please_select_message">
         Please select a previously-exported snapshot to import<br/>or drag it here.
      </div>
    </label>
    <div id="filepath"></div>
  </div>

<p>&nbsp;</p>

<div class="block_buttons">
  <input type="submit" id="submit-snapshot"/>
</div>

</form>

<?php
  require_once('inc/ajax-pending.inc');
  require_once('inc/ajax-failure.inc');
?>
</body>
</html>
