<?php @session_start(); ?>
<?php

// Receive POSTs to perform actions, return XML responses
//
// <action-response> is document root of reply, constructed from $_POST arguments
//
// This script will load the database config (i.e., perform a require_once of
// inc/data.inc) UNLESS the query or action name ends in ".nodata'.  Since
// inc/data.inc will force a page redirect if the database cannot be loaded, any
// queries or actions that potentially run before a database config file exists
// should be in include files that end in ".nodata.inc'.
require_once('inc/permissions.inc');
require_once('inc/authorize.inc');

require_once('inc/action-helpers.inc');

$json_out = array();

function json_out($key, $value) {
  global $json_out;
  $json_out[$key] = $value;
}

function json_success() {
  json_out('outcome', array('summary' => 'success',
                            'code' => 'success',
                            'description' => ''));
}

function json_failure($code, $description) {
  json_out('outcome', array('summary' => 'failure',
                            'code' => $code,
                            'description' => $description));
}

function json_sql_failure($sql) {
  global $db;
  $info = $db->errorInfo();
  json_failure('sql'.$info[0].'-'.$info[1], "$sql failed: $info[2] [EOM]");
}

function json_not_authorized() {
  json_failure('notauthorized', "Not authorized -- please see race coordinator.");
}

if (empty($_POST) && empty($_GET)) {
  echo '<!DOCTYPE html><html><head><title>Not a Page</title></head><body><h1>This is not a page.</h1></body></html>';
  exit(1);
}

$is_action = !empty($_POST);
$inc = $is_action ? @$_POST['action'] : @$_GET['query'];
$in_json = $inc != 'timer-message' && (strpos($inc, 'snapshot') === false)
           // These are to provide an error message for older derby-timer.jar
           && $inc != 'login' && $inc != 'roles';

if ($in_json) {
  header('Content-Type: application/json; charset=utf-8');
} else {
  header('Content-Type: text/xml; charset=utf-8');
  echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
}

if (substr($inc, -7) != '.nodata') {
  require_once('inc/data.inc');
  // We can close the session for writing except for these three that might
  // perform some write to $_SESSION.
  if (strstr($inc, 'ballot.get') === false &&
      strstr($inc, 'session.write') === false &&
      strstr($inc, 'role.login') === false) {
    session_write_close();
  }
}

if ($is_action) {
  $args = array();
  foreach ($_POST as $attr => $val) {
    $args[$attr] = $attr == 'password' ? '...' : $val;
  }
  json_out('action', $args);
  json_out('outcome', array('summary' => 'in-progress',
                            'code' => 'in-progress',
                            'description' => 'No outcome defined.'));
  if (isset($db) &&
      !($args['action'] == 'timer-message' && $args['message'] == 'HEARTBEAT') &&
      array_search($args['action'], array('role.login', 'vote.cast')) === false) {
    record_action($args);
  }
}

$prefix = $is_action ? 'action' : 'query';
if (!@include 'ajax/'.$prefix.'.'.$inc.'.inc') {
  if ($in_json) {
    json_failure('unrecognized', "Unrecognized $prefix: $inc");
  } else {
    start_response();
    echo '<failure code="unrecognized">Unrecognized '.$prefix.': '.$inc.'</failure>';
    end_response();
  }
}

if ($in_json) {
  if (empty($json_out)) {
    $json_out = new stdClass();
  }
  echo json_encode($json_out, JSON_PRETTY_PRINT | JSON_NUMERIC_CHECK);
  echo "\n";
}


?>
