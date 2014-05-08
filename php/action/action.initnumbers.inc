<?php
  // For a number of races I've been involved in, car numbers are
  // assigned at check-in, and it's fairly easy to forget to click
  // "passed" when you're fiddling with entering the car number for a
  // boy.  For this kind of race, the 99999 value gets noticed in the
  // renumber action, above, as a signal that renumberig should also
  // pass the inspection.
take_action('UPDATE RegistrationInfo SET carnumber = 99999 WHERE PassedInspection = 0',
			array());
?>