<?php

// $_POST['port'], empty for no choice

try {
  execute_once('UPDATE TimerSettings SET pending = :value'
               .' WHERE kind = :kind AND key = :key',
               array(':kind' => 'ports',
                     ':key' => 'ports',
                     ':value' => $_POST['port']));
} catch (PDOException $p) {
}

json_success();
?>
