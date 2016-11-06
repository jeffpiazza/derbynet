<!-- GENERATED FROM TEMPLATE -->
<div class="banner">
<?php
   if (isset($banner_title)) {
	 echo '<div class="banner_title">'.$banner_title.'</div>'."\n";
   }
?>
<?php
   if (!isset($no_back_button) || !$no_back_button) {
?>
  <div class="back_button" onclick="window.location='index.php'; return false;">
    <img src="img/back-button.png"/>
  </div>
<?php
   }
?>
<?php
   if (isset($_SESSION['role']) && $_SESSION['role']) {
	 echo '<div class="role">In '.$_SESSION['role'].' role.<br/><a href="login.php?logout">Log out</a></div>'."\n";
   }
?>
  <img class="logo" src="img/derbynet.png"/>
</div>
<div class="banner_version">@BRANCH@</div>