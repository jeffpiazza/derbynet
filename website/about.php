<?php @session_start();
require_once('inc/data.inc');
require_once('inc/authorize.inc');
?><html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>About DerbyNet</title>
<?php require('inc/stylesheet.inc'); ?>
<style type="text/css">
.ip_addr { border: 2px solid red;  padding: 2px; }

.phpinfo {background-color: #ffffff; color: #000000;}
.phpinfo, .phpinfo td, .phpinfo th, .phpinfo h1, .phpinfo h2 {font-family: sans-serif;}
.phpinfo pre {margin: 0px; font-family: monospace;}
.phpinfo a:link {color: #000099; text-decoration: none; background-color: #ffffff;}
.phpinfo a:hover {text-decoration: underline;}
.phpinfo table {border-collapse: collapse;}
.phpinfo .center {text-align: center;}
.phpinfo .center table { margin-left: auto; margin-right: auto; text-align: left;}
.phpinfo .center th { text-align: center !important; }
.phpinfo td, .phpinfo th { border: 1px solid #000000; font-size: 75%; vertical-align: baseline;}
.phpinfo h1 {font-size: 150%;}
.phpinfo h2 {font-size: 125%;}
.phpinfo .p {text-align: left;}
.phpinfo .e {background-color: #ccccff; font-weight: bold; color: #000000;}
.phpinfo .h {background-color: #9999cc; font-weight: bold; color: #000000;}
.phpinfo .v {background-color: #cccccc; color: #000000;}
.phpinfo .vr {background-color: #cccccc; text-align: right; color: #000000;}
.phpinfo img {float: right; border: 0px;}
.phpinfo hr {width: 600px; background-color: #cccccc; border: 0px; height: 1px; color: #000000;}
</style>

</head>
<body>
<?php $banner_title = 'About'; require('inc/banner.inc'); ?>
<h1>About DerbyNet</h1>

<p></p>

<?php
$addrs = gethostbynamel(gethostname());
if (count($addrs) == 0) {
  echo "<p>The local IP address for this server can't be determined.</p>\n";
} else {
  echo '<p>It looks like you can use ';
  // IIS apparently doesn't set REQUEST_URI.
  if (isset($_SERVER['REQUEST_URI'])) {
	$uri = dirname($_SERVER['REQUEST_URI']);
  } else {
	$uri = '/...';
  }

  $naddrs = count($addrs);
  for ($i = 0; $i < $naddrs; ++$i) {
    if ($i > 0) {
      echo ' or ';
    }
    echo "<span class='ip_addr'>http://".$addrs[$i].$uri."</span>";
  }
  echo " as the URL for connecting other local devices to this server.</p>\n";
}
?>

<p>Please include this page if you wish to report a bug, and
   contact me at <a href="mailto:bugs@jeffpiazza.org">bugs@jeffpiazza.org</a>.</p>

<h4>DerbyNet Revision</h4>
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
<div class="phpinfo">
<?php
ob_start();
 phpinfo();
$buf = ob_get_clean();
$start = strpos($buf, '<body>') + 6;
$stop = strpos($buf, '</body>');
echo substr($buf, $start, $stop - $start);
?>
</div>
</body>
</html>