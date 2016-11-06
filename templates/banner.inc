<!-- GENERATED FROM TEMPLATE -->
<div class="banner">
<?php if (!isset($no_back_button) || !$no_back_button) { ?>
  <div class="back_button">
    <form onsubmit="window.location='index.php'; return false;">
	  <input type="image" src="img/back-button.png" value="Back"/>
    </form>
  </div>
<?php
   }
   if (isset($banner_title)) {
	 echo '<div class="banner_title">'.$banner_title.'</div>'."\n";
   }
   if (isset($_SESSION['role']) && $_SESSION['role']) {
	 echo '<div class="role">In '.$_SESSION['role'].' role.<br/><a href="login.php?logout">Log out</a></div>'."\n";
   }
?>
  <img src="img/derbynet.png"/>
</div>
<div class="banner_version">@BRANCH@</div>