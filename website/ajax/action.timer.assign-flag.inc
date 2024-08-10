<?php

// $_POST['flag'], $_POST['value']

try {
  // The pending flag value will get transmitted to the timer as part of the
  // response to the next timer-message.
  execute_once('UPDATE TimerSettings SET pending = :value'
               .' WHERE kind = :kind AND key = :key',
               array(':kind' => 'flag',
                     ':key' => $_POST['flag'],
                     ':value' => $_POST['value']));
} catch (PDOException $p) {
}

json_success();

?>
