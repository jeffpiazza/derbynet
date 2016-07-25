<?php @session_start(); ?>
<?php

// Receive POSTs to perform actions, return XML responses
//
// <action-response> is document root of reply, constructed from $_POST arguments
//

// TODO bigtime: a bad or missing config file prevents directory queries

require_once('inc/permissions.inc');
require_once('inc/authorize.inc');

require_once('inc/action-helpers.inc');

if (!empty($_POST)) {
    header('Content-Type: text/xml; charset=utf-8');
    echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    if (substr($_POST['action'], -7) != '.nodata') {
      require_once('inc/data.inc');
    }
    if (!@include 'ajax/action.'.$_POST['action'].'.inc') {
        start_response();
	echo '<failure code="unrecognized">Unrecognized action: '.@$_POST['action'].'</failure>';
        end_response();
    }
} else if (!empty($_GET)) {
    header('Content-Type: text/xml; charset=utf-8');
    echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    if (substr($_GET['query'], -7) != '.nodata') {
      require_once('inc/data.inc');
    }
    if (!@include 'ajax/query.'.$_GET['query'].'.inc') {
        start_response();
        echo '<failure code="unrecognized">Unrecognized query: '.@$_GET['query'].'</failure>';
        end_response();
    }
} else {
    echo '<!DOCTYPE html><html><head><title>Not a Page</title></head><body><h1>This is not a page.</h1></body></html>';
}

?>