<?php

$roles_out = array();

foreach ($roles as $name => $details) {
  // Add a race_control="1" attribute if suitable role for the timer controller
  $roles_out[] = array('name' => $name,
                       'timer-message' => @$details['permissions'] & TIMER_MESSAGE_PERMISSION);
}

json_out('roles', $roles_out);
?>
