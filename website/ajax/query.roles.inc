<roles>
<?php
foreach ($roles as $name => $details) {
  // Add a race_control="1" attribute if suitable role for the timer controller
  echo "<role".((@$details['permissions'] & TIMER_MESSAGE_PERMISSION) == 0 ? ''
                : ' timer_message="1"').">"
      .htmlspecialchars($name, ENT_QUOTES, 'UTF-8')
      ."</role>\n";
}
?>
</roles>