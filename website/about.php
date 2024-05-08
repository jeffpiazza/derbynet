<?php
@session_start();
// NOTE: Loading inc/data.inc will cause a redirect to the set-up page if there's
// an issue with the database.  We want to avoid that, since an important use of the
// about page is to capture diagnostic information when troubleshooting...
require_once('inc/authorize.inc');
session_write_close();
// Note that schema_version doesn't load data.inc
require_once('inc/schema_version.inc');
require_once('inc/banner.inc');
require_once('inc/locked.inc');
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>About DerbyNet</title>
<?php require('inc/stylesheet.inc'); ?>
<script type="text/javascript">
window.onload = function() {
  var span = document.getElementById("useragent");
  if (span) {
    span.innerHTML = navigator.userAgent;
  }
};
</script>
<style type="text/css">

.advert {
  font-size: 18px;
  border: 5px solid #023882;
  background-color: #fcf0b5;
  padding: 20px;
  margin-bottom: 50px;
  margin-left: 50px;
  margin-right: 50px;
}

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

.main-body {
  margin-right: 350px;
  margin-left: 20px;
}

.capture-link {
  position: absolute;
  top: 100px;
  right: 50px;
  width: 250px;
  text-align: center;
  background-color: #408040;
}
.capture-link p {
    margin-top: 46px;
    margin-bottom: 46px;
}
.capture-link a {
  color: white;
}
</style>
</head>
<body>
<?php make_banner('About DerbyNet'); ?>
<div class="main-body">
<h1>About DerbyNet</h1>

<p class='advert'><b>DerbyNet</b> is the free, open-source, multi-screen race management system for Pinewood Derby-style racing.  It's used by packs and other groups all around the country, and around the globe!
Check us out <a href="http://jeffpiazza.github.io/derbynet/" target="_blank">on GitHub!</a></p>

<?php
$urls = preferred_urls();

if (count($urls) == 0 || empty($urls[0])) {
  echo "<p>The local IP address for this server can't be determined.</p>\n";
} else {
  echo '<p>It looks like you can use ';
  for ($i = 0; $i < count($urls); ++$i) {
    if ($i > 0) {
      echo ' or ';
    }
    echo "<span class='ip_addr'>".$urls[$i]."</span>";
  }
  echo " as the URL for connecting other local devices to this server.</p>\n";
}
?>

<p>Please include this page if you wish to report a bug, and
   contact me at <a href="mailto:bugs@derbynet.org">bugs@derbynet.org</a>.</p>

<p>Your browser's User Agent string is<br/><span id="useragent"></span>.</p>
<p>Your browser supports javascript version <span id="jsver">unspecified</span>.</p>

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
    $configdir = isset($_SERVER['DERBYNET_CONFIG_DIR']) ? $_SERVER['DERBYNET_CONFIG_DIR'] : 'local';
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

    if (have_permission(SET_UP_PERMISSION)) {
      echo "<div class='capture-link'>"
           ."<p class=''>Download Database Snapshot:<br/>"
           ."<a download='derbynet-".date('Ymd-His').".xml'"
           ." href='action.php?query=snapshot.get'>Complete</a>"
           ."<br/>or<br/>"
           ."<a download='derbynet-".date('Ymd-His').".xml'"
           ." href='action.php?query=snapshot.get&amp;clean'>Cleaned</a>"
           ."</p>"
           ."<p style='border-top: 1px solid white; padding: 26px; margin-bottom: 0px;'>"
           ."Download<br/><a download='derbynet.pref'"
           ." href='preferences.php'>Preferences</a>"
           ."</p></div>\n";
    }
  } catch (PDOException $p) {
    echo '<p>Can\'t determine schema version (expecting version '.expected_schema_version().')</p>'."\n";
  }
}
?>
<?php
  if (isset($db)) {
    function read_single_row($sql, $params = array(), $fetch = PDO::FETCH_NUM) {
      global $db;
      $rs = $db->prepare($sql);
      $rs->execute($params);
      $row = $rs->fetch($fetch);
      $rs->closeCursor();
      return $row;
    }

    function read_single_value($sql, $params = array(), $def = false) {
      $row = read_single_row($sql, $params);
      if ($row === false || $row[0] === null) {
        return $def;
      }

      return $row[0];
    }

    function read_raceinfo($key, $def = false) {
      return read_single_value('SELECT itemvalue FROM RaceInfo WHERE itemkey = :key',
                               array(':key' => $key), $def);
    }


    $timer = read_raceinfo('timer-type');
    if ($timer) {
      echo "<h4>Timer</h4>\n";
      echo "<p>";
      echo htmlspecialchars($timer, ENT_QUOTES, 'UTF-8');

      $timer_human = read_raceinfo('timer-humant');
      if ($timer_human) {
        echo " $timer_human";
      }
      $timer_ident = read_raceinfo('timer-ident');
      if ($timer_ident) {
        echo " ($timer_ident)";
      }
      echo "</p>\n";
    }
  }
?>
<h4>PHP Configuration Information</h4>
</div>

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

<script type="text/javascript">
  document.getElementById("jsver").textContent = "1.0";
</script>
<script language="Javascript1.1">
  document.getElementById("jsver").textContent = "1.1";
</script>
<script language="Javascript1.2">
  document.getElementById("jsver").textContent = "1.2";
</script>
<script language="Javascript1.3">
  document.getElementById("jsver").textContent = "1.3";
</script>
<script language="Javascript1.4">
  document.getElementById("jsver").textContent = "1.4";
</script>
<script language="Javascript1.5">
  document.getElementById("jsver").textContent = "1.5";
</script>
<script language="Javascript1.6">
  document.getElementById("jsver").textContent = "1.6";
</script>
<script language="Javascript1.7">
  document.getElementById("jsver").textContent = "1.7";
</script>
<script language="Javascript1.8">
  document.getElementById("jsver").textContent = "1.8";
</script>
<script language="Javascript1.9">
  document.getElementById("jsver").textContent = "1.9";
</script>
</html>
