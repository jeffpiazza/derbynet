<?php @session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/permissions.inc');
require_once('inc/roles.inc');

if (isset($_POST['name'])) {
  $name = $_POST['name'];
  $password = $_POST['password'];
 } else if (isset($_GET['name'])) {
  $name = $_GET['name'];
  $password = $_GET['password'];
 } else {
  $name = '';
  $password = '';
}


header('Content-Type: text/xml');

echo '<login-action>'."\n";

$role = $roles[$name];

if ($role) {
  if ($password == $role['password']) {
    $_SESSION['permissions'] = $role['permissions'];
    if ($password) {
      $_SESSION['role'] = $name;
	  echo '<success>'.$_SESSION['role'].'</success>'."\n";
	} else {
      // Despite the unset, it appears $_SESSION['role'] will appear
      // as an empty string in subsequent accesses.
      unset($_SESSION['role']);
	  echo '<success/>'."\n";
	}
  } else {
    echo '<failure>Incorrect password</failure>'."\n";
  }
} else {
  echo '<failure role="'.$name.'">No such role: '.$name.'</failure>'."\n";
}
echo '</login-action>'."\n";
?>