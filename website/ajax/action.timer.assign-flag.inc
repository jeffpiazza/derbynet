<?php

// $_POST['flag'], $_POST['value']

try {
read_single_value('UPDATE TimerSettings SET pending = :value'
                  .' WHERE kind = :kind AND key = :key',
                  array(':kind' => 'flag',
                        ':key' => $_POST['flag'],
                        ':value' => $_POST['value']));
} catch (PDOException $p) {
}

echo "<success/>\n";

?>