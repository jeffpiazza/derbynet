<?php
?>
<html>
<head>
<title>TEST: Create database</title>
</head>
<body>
<?php


require_once('../data.inc');

require_once('create-database.'.$dbtype.'.inc');

	if (false) {
	  $rs = $db->query('SELECT COUNT(*) FROM RegistrationInfo');
	  $result = $rs->fetch();
	  // Should be empty:
	  if ($result[0] > 0) {
		echo '<h2>RegistrationInfo table is not empty!</h2>';
		echo '</body></html>';
		exit(1);
	  }
	  $rs->closeCursor();
	  // Get a new connection, because the old one will keep a lock on
	  // RegistrationInfo.  No way to close the old one explicitly?
	  $db = new PDO('odbc:DSN=gprm;Exclusive=NO','','');
	}

	foreach ($sql_script as $stmt) {
	  echo '<p>Executing '.$stmt.'</p>'."\n";
	  $db->exec($stmt);
	}
	echo '<h2>Database script completed!</h2>';

?>
</body>
</html>
