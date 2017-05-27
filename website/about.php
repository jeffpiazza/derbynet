<?php @session_start();
// NOTE: Loading inc/data.inc will cause a redirect to the set-up page if there's
// an issue with the database.  We want to avoid that, since an important use of the
// about page is to capture diagnostic information when troubleshooting...
require_once('inc/authorize.inc');
// Note that schema_version doesn't load data.inc
require_once('inc/schema_version.inc');
require_once('inc/banner.inc');
?><!DOCTYPE html>
<html>
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
<?php make_banner('About DerbyNet'); ?>
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
    $git_hash = @file_get_contents('inc/generated-commit-hash.inc');
    if ($version === false) {
      echo "<p>No version found.</p>\n";
    } else {
      echo "<p>This is revision <b>".$version."</b>, built on ".$build_date.".<br/>\n";
      echo "(".$git_hash.")</p>\n";
    }
?>
<?php
    $failed = false;
    set_error_handler(function() { global $failed; $failed = true; }, E_WARNING);
    date_default_timezone_get();
    restore_error_handler();
    if ($failed) {
      echo "<h3>Time zone not set!</h3>\n";
      echo "<p>You need to set the date.timezone setting in the php.ini file!</p>\n";
    }
?>
<h4>Database Configuration</h4>
<?php
    $configdir = isset($_SERVER['CONFIG_DIR']) ? $_SERVER['CONFIG_DIR'] : 'local';
    if (have_permission(SET_UP_PERMISSION)) {
      $config_content = @file_get_contents($configdir.DIRECTORY_SEPARATOR.'config-database.inc',
                                           /* use_include_path */ true);
      if ($config_content === false) {
        echo '<p>Database configuration file, '.
            htmlspecialchars($configdir.DIRECTORY_SEPARATOR.'config-database.inc',
                             ENT_QUOTES, 'UTF-8').
            ', could not be opened.'.
            '</p>'."\n";
      } else {
        echo "<pre>\n";
        echo htmlspecialchars($config_content, ENT_QUOTES, 'UTF-8');
        echo "</pre>\n";
      }
    } else {
      echo "<p>Database configuration information is available if you are logged in.</p>\n";
    }
?>
<?php
// Try setting up the database, but it's OK if it doesn't work out
try {
  @include($configdir.DIRECTORY_SEPARATOR."config-database.inc");
} catch (PDOException $p) {
}

if (isset($db)) {
  $db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
  echo "<h4>Database Schema</h4>\n";
  try {
    // Can't use schema_version(), because it depends on functions from data.inc
    $rs = $db->prepare('SELECT itemvalue FROM RaceInfo WHERE itemkey = :key');
    $rs->execute(array(':key' => 'schema'));
    $row = $rs->fetch(PDO::FETCH_NUM);
    $rs->closeCursor();
    $schema_version = $row === false ? false : $row[0];
    echo '<p>Schema version '.$schema_version.' (expecting version '.expected_schema_version().')</p>'."\n";
  } catch (PDOException $p) {
    echo '<p>Can\'t determine schema version (expecting version '.expected_schema_version().')</p>'."\n";
  }
}
?>
<h4>PHP Configuration Information</h4>
<div class="phpinfo">
<?php
ob_start();
 @phpinfo();
$buf = ob_get_clean();
$start = strpos($buf, '<body>') + 6;
$stop = strpos($buf, '</body>');
echo substr($buf, $start, $stop - $start);
?>
</div>
</body>
</html>