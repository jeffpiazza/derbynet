<?php @session_start(); ?>
<?php

// Receive POSTs from the checkin page to perform check-in actions,
// return XML responses

require_once('inc/data.inc');
require_once('inc/permissions.inc');
require_once('inc/authorize.inc');


header('Content-Type: text/xml');

require_once('action/helpers.inc');

if (!@include 'action/action.'.$_POST['action'].'.inc') {
    echo '<checkin><failure>Unrecognized post: '.$_POST['action'].'</failure></checkin>';
}

?>