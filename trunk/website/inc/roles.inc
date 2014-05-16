<?php
@session_start();

/*
  This is a one-time initialization script, more or less, for setting up permission "roles".
  YOU MAY WELL NEED TO EDIT THIS FILE!!
  This is the file to edit to control:
    - What roles you need
    - Passwords for roles that need to log in
    - Who get to see (and do) what.
 */

require_once('data.inc');
require_once('permissions.inc');

  // Define your roles and permissions here, and assign passwords.
  // Consider these just to be examples.  Upon first connection, a
  // role without a password, if any, will be selected, and the
  // contained permissions assigned.  There should only be one such
  // role, and the permissions associated with that role are what a
  // non-logged-in user can do.

$roles = array('' => 
	       array('password' => '',
		     'permissions' =>
		     VIEW_RACE_RESULTS_PERMISSION),
	       'RaceCrew' =>
	       array('password' => 'murphy',
		     'permissions' =>
		     VIEW_RACE_RESULTS_PERMISSION | VIEW_AWARDS_PERMISSION
		     | CHECK_IN_RACERS_PERMISSION | REVERT_CHECK_IN_PERMISSION
		     | RENUMBER_CAR_PERMISSION | REGISTER_NEW_RACER_PERMISSION),
	       'RaceCoordinator' =>
	       array('password' => 'doyourbest',
		     'permissions' => -1)
	       );

?>