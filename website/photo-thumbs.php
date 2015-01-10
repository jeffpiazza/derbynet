<?php
session_start();
require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_permission(ASSIGN_RACER_IMAGE_PERMISSION);

require_once('inc/photo-config.inc');

function scan_directory($directory, $pattern) {
  $files = array();

  $dh  = @opendir($directory);
  if ($dh !== false) {
      while (($filename = readdir($dh)) !== false) {
          if (preg_match($pattern, $filename) && is_file($directory.DIRECTORY_SEPARATOR.$filename)) {
              $files[] = $filename;
          }
      }
      closedir($dh);
  }

  return $files;
}

$allfiles = scan_directory(PhotoRender::lookup('original')->directory(),
						   "/(jpg|jpeg|png|gif|bmp)/i");

// TODO: line-height?  "End of photos" text aligns with thumbnail image bottom.
// *** Both div's are overhanging the bottom by the amount taken up by the banner and refresh button!
// *** height=100% could be at issue.
//
// TODO: Separate requests to bind or remove photo.
//
// TODO: Allow for more than one kind of associated image.
?>
<!DOCTYPE HTML>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Assign Racer Photos</title>
<meta name="viewport" content="width=device-width, initial-scale=1"/>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/photo-thumbs.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery-ui-1.10.4.min.js"></script>
<script type="text/javascript" src="js/jquery.ui.touch-punch.min.js"></script>
<script type="text/javascript" src="js/dashboard-ajax.js"></script>
<script type="text/javascript" src="js/checkin.js"></script>
<script type="text/javascript" src="js/photo-thumbs.js"></script>
</head>
<body>
<?php $banner_title = 'Racer Photos'; require('inc/banner.inc'); ?>

<div class="block_buttons">
<form method="link">
  <input type="submit" value="Refresh"/>
</form>
</div>

<div class="body-wrapper">

<div class="thumblist">
<ul data-role="listview" class="ui-listview">
<?php
require_once('inc/data.inc');
$racers_by_photo = array();
$stmt = $db->query('SELECT racerid, lastname, firstname, imagefile, carnumber, class'
				   .' FROM RegistrationInfo'
				   .' INNER JOIN Classes'
				   .' ON RegistrationInfo.classid = Classes.classid'
				   .' ORDER BY lastname, firstname');
foreach ($stmt as $rs) {
  $raw_imagefile = $rs['imagefile'];
  $racer = array('firstname' => $rs['firstname'],
				 'lastname' => $rs['lastname'],
				 'class' => $rs['class'],
				 'racerid' => $rs['racerid'],
				 'imagefile' => $raw_imagefile);


  $css_classes = 'ui-li-static ui-li-has-thumb';
  if ($raw_imagefile !== null && $raw_imagefile !== "") {  // If there's an associated photo...
	$image_filename = basename($raw_imagefile);
	$racers_by_photo[$image_filename] = $racer;
	if (array_search($image_filename, $allfiles) === false) {
	  $css_classes .= ' without-photo lost_photo';
	}
  } else {
	$css_classes .= ' without-photo';
  }

  echo '<li data-racer-id="'.$racer['racerid'].'" '
           .' class="'.$css_classes.'"'
    	   .'>';

  if ($raw_imagefile != '') {
	echo "\n".'<img class="assigned"'
      .' data-image-filename="'.htmlspecialchars($image_filename, ENT_QUOTES, 'UTF-8').'"'
    .' src="'.PhotoRender::lookup('tiny')->url($image_filename).'"/>';
  }
  echo htmlspecialchars($rs['firstname'].' '.$rs['lastname'], ENT_QUOTES, 'UTF-8');
  echo '<p><strong>'.$rs['carnumber'].':</strong> '.htmlspecialchars($rs['class'], ENT_QUOTES, 'UTF-8').'</p>';
  echo '</li>'."\n";
}

?>
</ul>

</div>

<div class="photothumbs">
<?php
foreach ($allfiles as $imagefile) {
  echo '<div class="thumbnail'.(isset($racers_by_photo[$imagefile]) ? ' hidden' : '').'">';
  echo '<a href="photo-crop.php?name='.urlencode($imagefile).'">';
  echo '<img class="unassigned-photo"'
      .' data-image-filename="'.htmlspecialchars($imagefile, ENT_QUOTES, 'UTF-8').'"'
      .' src="'.PhotoRender::lookup('thumb')->url($imagefile).'"/>';
  echo '</a>';
  // echo $imagefile;
  echo '</div>'."\n";
}

if (empty($allfiles)) {
    echo '<h2>There are no photos in the photo directory yet.</h2>';
}
?>
</div>

</div>
<div id="ajax_working" class="hidden">
  <span id="ajax_num_requests">0</span> request(s) pending.
</div>
</body>
</html>
