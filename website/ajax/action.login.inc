<?php

if (isset($_POST['name'])) {
  $name = $_POST['name'];
  $password = $_POST['password'];
  // Don't want the password parroted back in the response.
  $_POST['password'] = '...';
 } else {
  $name = '';
  $password = '';
}

start_response();

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

end_response();
?>