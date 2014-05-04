<?php

  // Want to do a slide for background while presenting an award.
  //
  // Should have racer's head shot, car photo, car number, racer's
  // name, and award name.
  //
  // "Cropped" directory has the cropped head shot at "working" size,
  // maximum 900 pixels.  Sample images happen to be a little smaller,
  // 600px or so.  Probably want a separate, explicit directory for
  // these correctly-sized-for-award-page head shots.
  //
  // For car photo... well, we don't really know where to find that.
  // Photo-fetch could be easily extended; the main problem is that
  // the file names for the car photos would have nothing to do with
  // the head shot file names.  Need a separate table in the database
  // to track.  Relatively little code needs to be conditionalized,
  // but it's a little ugly.
  //
  // Also need a way to transmit the car photos as they arrive on the
  // slideshow machine (mac).  Ethernet between laptops and router for
  // more bandwidth.  Should be simple enough for one machine to poll
  // and push/pull the photo to the other.
  //
  // (2MB photo is what I got for cars.  At 54Mbits/sec, 2MB = 16Mbits
  // would transmit in a fraction of a second.  ** Worth timing how
  // long a photo transfer takes in practice. ** Probably depends a
  // lot on how frequently the laptop polls the camera card.
  //
  // Page doesn't load all that smoothly (over VPN), which kind of
  // kills the timing of the fade-in.
  //
  // From sample GIMP image, 1920x1080 image:
  // Photos take top 700px, text underneath
  // 140px Palatino Bold font
?>
<html>
<head>
<title>Award Presentation</title>
<link rel="stylesheet" type="text/css" href="jquery-mobile/jquery.mobile-1.4.2.css"/>
<?php require('stylesheet.inc'); ?>
<script type="text/javascript" src="jquery.js"></script>
<script type="text/javascript">
$(document).ready(function() {
					setTimeout(function () {
								 $(".award-trophy").fadeIn();
							   }, 3000);
				  });
</script>
<style>
.top-photos {
  height: 700px;
}

table { 
 width: 100%;
 border: 0px;
 }
td { border: 0px; }
body { margin: 0px; }
.award-racer-photo-left {
  float: left;
  width: 50%;
  height: 100%;
}
.award-racer-photo-left img {
  display: block;
  margin-left: auto;
  margin-right: auto;
}
.award-car-photo-right {
   width: 50%
   height: 100%;
   vertical-align: center;
}
.award-car-photo-right img {
  display: block;
  margin-left: auto;
  margin-right: auto;

}
.award-racer-name {

 height: 380px;
   font-family: Palatino;
   font-size: 140px;
}
</style>
</head>
<body>

<table class="award-background">
<tr class="top-photots">
<td class="award-racer-photo-left">
    <img src="photo-fetch.php/cropped/Bender-Zack.jpg"/>
</td>
<td class="award-car-photo-right">
    <img src="Bender-car.JPG"/>
</td>
</tr>
<tr>
<td colspan="2" class="award-racer-name">
    <p><span class="car-number">201</span> -
       <span class="firstname">Zack</span>
       <span class="lastname">Bender</span>
    </p>
    <p class="award-trophy fade-in">First Place, Wolves</p>
</td>
</tr>
</table>
</body>
</html>