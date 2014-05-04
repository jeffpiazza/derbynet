<?php @session_start(); ?>
<?php
require_once('data.inc');
require_once('permissions.inc');
require_once('authorize.inc');
?>
<html>
<head>
<title>Please Log In</title>
<?php require('stylesheet.inc'); ?>
<script type="text/javascript" src="jquery.js"></script>
<script type="text/javascript">

function readystate_handler() {
    // "this" = XMLHttpRequest
    // attribute DOMString responseText
    // attribute Document responseXML
    // attribute unsigned short status
    // attribute DOMString statusText

    // Document.documentElement
    if (this.readyState == this.DONE) {
		//alert("Ready: " + this.readyState + ", " + this.status + ", " + this.responseText);
		//alert("responseType: [" + this.responseType + "]"); 
		var response = $(this.responseXML);
		var succ = response.find("success");
		if (succ && succ.size() > 0) {
			//alert("Success: " + response.find("success").text());
			window.location.href = 'index.php';
		} else {
			var fail = response.find("failure");
			if (fail && fail.size() > 0) {
				alert("Login fails: " + fail.text());
			} else {
				alert("Unrecognized XML: " + this.responseXML);
			}
		}
	}
}

function handle_login(role, pwd) {
	 var xmlhttp = new XMLHttpRequest();
	 xmlhttp.open("POST", "login-action.php", /*async*/true);
	 xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
	 xmlhttp.onreadystatechange = readystate_handler;
	 xmlhttp.send("name=" + role + "&password=" + pwd);
}

function show_password_form(name) {
	document.getElementById("name_for_password").value = name;
	$("#password_form").removeClass("hidden");
	// Giving immediate focus doesn't work, because the element is still invisible.
	// Waiting 100ms is clumsy, but reasonably effective.
	setTimeout(function() { 
			document.getElementById('pw_for_password').focus(); }, 100);
}

function handle_password_submission(form) {
	var role = document.getElementById("name_for_password").value;
	var pwd = document.getElementById("pw_for_password").value;
	handle_login(role, pwd);
}
</script>
</head>
<body<?php if (isset($_GET['logout'])) echo ' onload="handle_logout()"'; ?>>
<div class="block_buttons">
<form id="password_form" class="floatform hidden"
      onsubmit="handle_password_submission(this); return false;">
  <input type="hidden" name="name" id="name_for_password" value=""/>
  <p>Enter password:
     <input type="password" name="pw" id="pw_for_password"/>
  </p>
  <input type="submit" value="Submit"/>
  <input type="button" value="Cancel"
      onclick='$("#password_form").addClass("hidden");'/>
</form>
</div>
<?php
require('banner.inc');

echo '<div class="index_background">'."\n";
echo '<div class="block_buttons">'."\n";
echo '<legend>Please choose a role:</legend>'."\n";

foreach ($roles as $name => $details) {
  if (!$details['password']) {
    $logout = $name;
  } else {
    echo '<input type="button" value="'.$name.'" onclick=\'show_password_form("'.$name.'");\'/>'."\n";
  }
}

if (isset($_SESSION['role'])) {
  echo '<br/>'."\n";
  //echo '<h2 onclick=\'handle_login("")\'>'.$logout.'</h2>'."\n";
  echo '<input type="button" value="Log out" onclick=\'handle_login("", "");\'/>'."\n";
  echo '<script type="text/javascript">'."\n";
  echo 'function handle_logout() {'."\n";
  echo '   handle_login("", "");'."\n";
  echo '}'."\n";
  echo '</script>'."\n";
 }
echo '</div>'."\n";
echo '</div>'."\n";
?>
</body>
</html>