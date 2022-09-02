<!DOCTYPE html>
<html>
<?php
require_once('inc/banner.inc');
require_once('inc/photo-config.inc');
?><head>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
    <title>DerbyNet Slideshow</title>
    <script type="text/javascript" src="js/jquery.js"></script>
<?php require('inc/stylesheet.inc'); ?>
    <link rel="stylesheet" type="text/css" href="css/kiosks.css"/>
    <link rel="stylesheet" type="text/css" href="css/slideshow.css"/>
    <script type="text/javascript" src="js/slideshow.js"></script>
  </head>

  <body>
  <?php make_banner('Slideshow'); ?>
  <div id="photo-background" class="photo-background">
    <div class="current">
      <img class='mainphoto' onload='mainphoto_onload(this)'
           src='slide.php/title'/>
    </div>
    <div class="next">
      <img class='mainphoto' onload='mainphoto_onload(this)'
           src='slide.php/title'/>
    </div>
  </div>

  </body>
</html>
