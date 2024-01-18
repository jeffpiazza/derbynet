<!DOCTYPE html>
<html>
<?php
require_once('inc/banner.inc');
require_once('inc/photo-config.inc');
?><head>
    <meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
    <title>DerbyNet Slideshow</title>
    <script type="text/javascript" src="js/jquery.js"></script>
<?php if (isset($as_kiosk)) require_once('inc/kiosk-poller.inc'); ?>
<?php require('inc/stylesheet.inc'); ?>
    <link rel="stylesheet" type="text/css" href="css/kiosks.css"/>
    <link rel="stylesheet" type="text/css" href="css/slideshow.css"/>
    <script type="text/javascript">
       var g_kiosk_parameters = <?php
            echo isset($as_kiosk) ? json_encode(kiosk_parameters()) : "{}";
       ?>;
    </script>
    <script type="text/javascript" src="js/slideshow.js"></script>
  </head>

  <body>
  <?php if (!isset($as_kiosk)) make_banner('Slideshow'); ?>
  <div id="photo-background" class="photo-background">
    <div class="next">
      <img class='mainphoto' onload='mainphoto_onload(this)'
           src='slide.php/title'/>
    </div>
  </div>

  <?php if (isset($as_kiosk)) require('inc/ajax-failure.inc'); ?>
  </body>
</html>
