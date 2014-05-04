<?php
session_start();
require_once('data.inc');
require_once('authorize.inc');
require_permission(ASSIGN_RACER_IMAGE_PERMISSION);

// TODO: Nothing prevents trying to assign more than one picture to a
// single racer.  (Last one wins, but the UI doesn't update for the
// displaced image.)

require_once('photo-config.inc');

function scan_directory($directory, $pattern) {
  $files = array();
  $dh  = opendir($directory);
  while (($filename = readdir($dh)) !== false) {
	if (preg_match($pattern, $filename)) {
	  $files[] = $filename;
	}
  }
  closedir($dh);

  return $files;
}

$allfiles = scan_directory($photoOriginalsDirectory,
						   "/(jpg|jpeg|png|gif|bmp)/i");
?>

<html>
<head>
<title>Thumbnail Images</title>
<?php require('stylesheet.inc'); ?>
<script type="text/javascript" src="jquery.js"></script>
<script type="text/javascript" src="checkin.js"></script>
</head>
<body>
<?php $banner_title = 'Racer Photos'; require('banner.inc'); ?>
<div class="thumblist">
  <h3>Racers without photos</h3>
<ul>
<?php
require_once('data.inc');
$rs = odbc_exec($conn, 'select racerid, lastname, firstname, imagefile'
				.' from registrationinfo'
				.' order by lastname, firstname');

$racers_by_id = array();
$racers_by_photo = array();
$racers_by_name = array();
while (odbc_fetch_row($rs)) {
  $racer = array('firstname' => odbc_result($rs, 'firstname'),
				 'lastname' => odbc_result($rs, 'lastname'),
				 'racerid' => odbc_result($rs, 'racerid'),
				 'imagefile' => odbc_result($rs, 'imagefile'));
  $racers_by_id[odbc_result($rs, 'RacerID')] = $racer;
  $racers_by_name[odbc_result($rs, 'lastname').', '.odbc_result($rs, 'firstname')] = $racer;
  echo '<li id="missing-'.$racer['racerid'].'"';
  if (odbc_result($rs, 'imagefile') !== "") {
	$img = pathinfo(odbc_result($rs, 'imagefile'), PATHINFO_BASENAME);
	$racers_by_photo[$img] = $racer;
	if (array_search($img, $allfiles) === false) {
	  echo ' class="lost_photo"';
	} else {
	  echo ' class="hidden"';
	}
  }
  echo '>'.odbc_result($rs, 'FirstName').' '.odbc_result($rs, 'LastName').'</li>';
}

foreach ($racers_by_photo as $photo => $racer) {
  
}
?>
</ul>
</div>
<div class="photothumbs">
<?php
foreach ($allfiles as $imagefile) {
  echo '<div class="thumbnail">';
  echo '<a href="photo-crop.php?name='.urlencode($imagefile).'">';
  echo '<img src="photo-fetch.php/thumb/';
  $thumbfile = $photoThumbsDirectory.'\\'.$imagefile;
  if (file_exists($thumbfile)) {
	echo @filemtime($thumbfile).'/';
  }
  echo urlencode($imagefile);
  echo '"/>';
  echo '</a>';

  if (isset($racers_by_photo[$imagefile])) {
	$r = $racers_by_photo[$imagefile]['racerid'];
  } else {
	$r = 0;
  }

  echo '<select name="'.$imagefile.'" onchange="changeRacerPhoto('.$r.', this)">';
  echo '<option value="0"'
       .($r ? '' : ' selected="1"')
       .'>--</option>'."\n";

  foreach ($racers_by_name as $racer) {
	echo '<option value="'.$racer['racerid'].'"';
	if ($racer['racerid'] == $r) {
	  echo ' selected="1"';
	}
	echo '>';
	echo $racer['firstname'].' '.$racer['lastname'].'</option>'."\n";
  }
  echo '</select>';

  echo '</div>'."\n";
}
?>
</div>
<div class="block_buttons">
<form method="link" action="photo-thumbs.php">
  <input type="submit" value="Refresh"/>
</form>
</div>
<script type="text/javascript">
function changeRacerPhoto(previous, sel) {
  // POST 'racer' => racerid, 'previous' => racerid, 'photo' => image file path
  var photo = sel.name;
  var racer = sel.options[sel.selectedIndex].value;

  // console.log('changeRacerPhoto: changing from ' + previous + ' to ' + racer);

  $("#missing-" + previous).removeClass("hidden");
  $("#missing-" + racer).addClass("hidden");
  // Remove old change handler(s) and attach new one reflecting current setting,
  // in case the selection gets changed again.
  $(sel).attr('onchange', '')
        .off('change')
        .change(function() {
				  changeRacerPhoto(racer, this);
				});

   var xmlhttp = new XMLHttpRequest();
   xmlhttp.open("POST", g_checkin_action_url, /*async*/true);
   xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
   xmlhttp.onreadystatechange = readystate_handler;
   xmlhttp.send("action=photo&racer=" + racer
				+ "&previous=" + previous
				+ "&photo=" + encodeURIComponent("<?php echo addcslashes($photoThumbsDirectory.'\\', '\\\"'); ?>" + photo));
   ajax_add_request();
}
</script>
</body>
<?php odbc_close($conn); ?>
</html>
