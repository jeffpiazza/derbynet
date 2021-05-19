<?php

// $_POST['port'], empty for no choice

try {
  read_single_value('UPDATE TimerSettings SET pending = :value'
                    .' WHERE kind = :kind AND key = :key',
                    array(':kind' => 'ports',
                          ':key' => 'ports',
                          ':value' => $_POST['port']));
} catch (PDOException $p) {
}

json_success();
?>
