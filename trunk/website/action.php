<?php @session_start(); ?>
<?php

// Receive POSTs to perform actions, return XML responses
//
// <action-response> is document root of reply, constructed from $_POST arguments
//
require_once('inc/data.inc');
require_once('inc/permissions.inc');
require_once('inc/authorize.inc');

require_once('inc/action-helpers.inc');

if (!empty($_POST)) {
    header('Content-Type: text/xml');
    if (!@include 'ajax/action.'.$_POST['action'].'.inc') {
        start_response();
        echo '<failure code="unrecognized">Unrecognized action: '.@$_POST['action'].'</failure>';
        end_response();
    }
} else if (!empty($_GET)) {
    header('Content-Type: text/xml');
    if (!@include 'ajax/query.'.$_GET['query'].'.inc') {
        start_response();
        echo '<failure code="unrecognized">Unrecognized query: '.@$_GET['query'].'</failure>';
        end_response();
    }
} else {
    echo '<html><head><title>Not a Page</title></head><body><h1>This is not a page.</h1></body></html>';
}

?>