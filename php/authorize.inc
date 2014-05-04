<?php
require_once('data.inc');
require_once('permissions.inc');
require_once('roles.inc');

if (!isset($_SESSION['permissions'])) {
  $_SESSION['permissions'] = $roles['']['permissions'];
}

// This only works where a header can appear...
function require_permission($perm) {
  if (($_SESSION['permissions'] & $perm) == 0) {
    header('Location: login.php');
    exit(0);
  }
}

function have_permission($perm) {
  return ($_SESSION['permissions'] & $perm) != 0;
}
?>
