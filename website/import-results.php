<?php @session_start();
require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_permission(SET_UP_PERMISSION);
?><html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Import Results</title>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/import-roster.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript">
// Turn off jQuery Mobile page loading hijack functionality, which
// tries to perform page loads via ajax, a "service" we don't really want.
// Fortunately, it's possible to turn that stuff off.
$(document).bind("mobileinit", function() {
				   $.extend($.mobile, {
					 ajaxEnabled: false
					 });
				 });
</script>
<script type="text/javascript" src="js/jquery-ui-1.10.4.min.js"></script>
<script type="text/javascript" src="js/jquery.ui.touch-punch.min.js"></script>
<script type="text/javascript" src="js/jquery.mobile-1.4.2.min.js"></script>
<script type="text/javascript" src="js/dashboard-ajax.js"></script>
<script type="text/javascript" src="js/checkin.js"></script>
<script type="text/javascript" src="js/jquery.csv.js"></script>
<script type="text/javascript" src="js/import-results.js"></script>
</head>
<body>
<?php
$banner_title = 'Import Results';
require('inc/banner.inc');

try {
  $racers = read_single_value("SELECT COUNT(*) FROM RegistrationInfo", array());
} catch (PDOException $p) {
  $racers = -1;
}

?><div class="import_roster">

<div id="top_matter">

<div id="new_ranks">
</div>

<div id="controls">
  <div id="meta"></div>

  <?php if ($racers > 0) { ?>
    <p>There are already <?php echo $racers; ?> racer(s) registered.
    Re-initialize the database schema if you're trying to start fresh.</p>
  <?php } else if ($racers < 0) { ?>
    <p>The RegistrationInfo table could not be read: you should probably finish setting up the database.</p>
  <?php } ?>

  <form method="link" action="database-setup.php">
    <input type="submit" data-enhanced="true" value="Set Up Database"/>
  </form>

  <form method="link">
    <input type="submit" id="start_over_button" data-enhanced="true" class="hidden" value="Start Over"/>
  </form>

  <div id="submit_message">Please select or drag a file to import.</div>

</div>

<div class="file_target">
  <input type="file" id="csv_file" name="csv_file"/>
</div>

  <div id="import_button_div">
    <input type="button" id="import_button" data-enhanced="true" value="Import Race Results"/>
  </div>

<!-- top_matter --></div>

</div>

<?php require_once('inc/ajax-pending.inc'); ?>
</body>
</html>