<?php

// $_POST['device'], empty for no choice

try {
  read_single_value('INSERT INTO TimerSettings(kind, pending)'
                    .' VALUES(:kind, :value)',
                    array(':kind' => 'pending-device',
                          ':value' => $_POST['device']));
} catch (PDOException $p) {
}

json_success();
?>
