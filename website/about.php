<?php @session_start();
require_once('inc/data.inc');
require_once('inc/authorize.inc');
?><html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>About Web Race Manager</title>
<?php require('inc/stylesheet.inc'); ?>
</head>
<body>
<?php $banner_title = 'About'; require('inc/banner.inc'); ?>
<h1>About Web Race Manager</h1>

<p></p>

<p>Please include this page if you wish to report a bug, and
   contact me at <a href="mailto:bugs@jeffpiazza.org">bugs@jeffpiazza.org</a>.</p>

<h4>Web Race Manager Revision</h4>
<?php 
    $version = @file_get_contents('inc/generated-version.inc');
    $build_date = @file_get_contents('inc/generated-build-date.inc');
    if ($version === false) {
      echo "<p>No version found.</p>\n";
    } else {
      echo "<p>This is revision <b>".$version."</b>, built on ".$build_date.".</p>\n";
    }

    if (have_permission(SET_UP_PERMISSION)) {
      echo "<h4>Database Connection</h4>\n";

      echo "<pre>\n";
      echo htmlspecialchars(file_get_contents('local'.DIRECTORY_SEPARATOR.'config-database.inc',
                                              /* use_include_path */ true),
                            ENT_QUOTES, 'UTF-8');
      echo "</pre>\n";
    } else {
      echo "<h4>Please Log In</h4>\n";
      echo "<p>Database configuration information is available if you are logged in.</p>\n";
    }
?>
<h4>PHP Configuration Information</h4>
<?php phpinfo(); ?>
</body>
</html>