<?php @session_start(); ?>
<?php

// Receive POSTs from the checkin page to perform check-in actions,
// return XML responses

require_once('data.inc');
require_once('permissions.inc');
require_once('authorize.inc');


header('Content-Type: text/xml');

require_once('action/helpers.inc');

if ($_POST['action'] == 'pass') {
	require('action/action.pass.inc');
} else if ($_POST['action'] == 'xbs') {
	require('action/action.xbs.inc');
} else if ($_POST['action'] == 'renumber') {
	require('action/action.renumber.inc');
} else if ($_POST['action'] == 'classchange') {
	require('action/action.classchange.inc');
} else if ($_POST['action'] == 'photo') {
	require('action/action.photo.inc');
} else if ($_POST['action'] == 'initaudit') {
	require('action/action.initaudit.inc');
} else if ($_POST['action'] == 'initnumbers') {
	require('action/action.initnumbers.inc');
} else {
    echo '<checkin><failure>Unrecognized post: '.$_POST['action'].'</failure></checkin>';
}
?>