<?php
session_start();
require_once('data.inc');
require_once('authorize.inc');
require_permission(ASSIGN_RACER_IMAGE_PERMISSION);

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

// TODO: Use a data- property/attribute to match hidden image with
// discarded one.  (Sequentially assign image number?)
//
// TODO: line-height?
//
// TODO: Separate requests to bind or remove photo.
?>
<!DOCTYPE HTML>
<html class="fullview">
<head>
<title>Thumbnail Images</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<link rel="stylesheet" type="text/css" href="jquery-mobile/jquery.mobile-1.4.2.css"/>
<?php require('stylesheet.inc'); ?>
<script type="text/javascript" src="jquery.js"></script>
<script type="text/javascript" src="jquery-ui-1.10.4/ui/jquery-ui.js"></script>
<script type="text/javascript" src="jquery.ui.touch-punch.min.js"></script>
<script type="text/javascript" src="checkin.js"></script>
<script type="text/javascript">
  $(function() {
	  $(".droppable").droppable({
		scope: "assign",
		hoverClass: "droppableHover",
		drop: function(event, ui) {
			// $(this).addClass("hidden");
			// $(this).attr("id") is "missing-" plus the racerid
			// photo-fetch.php/thumb/';
			// Wow, is this fragile...
			// jquery.remove() to remove the element from the page.
			// .parent()

			var photo_base_name = ui.draggable.attr("src").substring(22);
			var racer_id = $(this).attr("id").substring(8);
			
			console.log('drop: this.attr(id) = ' + $(this).attr("id"));
			console.log('drop: draggable.attr(src) = ' + ui.draggable.attr("src"));
			console.log('drop: draggable.parent().prop(tagName) = ' + ui.draggable.parent().prop('tagName'));
			console.log('drop: draggable.parent().parent().prop(tagName) = ' + ui.draggable.parent().parent().prop('tagName'));
			changeRacerPhotoAjax(0, racer_id, photo_base_name);
			ui.draggable.parent().parent().remove();
			console.log('drop: photo_base_name = '.photo_base_name);
			console.log('drop: urlencode(photo_base_name) = ' + encodeURIComponent(photo_base_name));
			// TODO: Needs the "click to revert" functionality added
			$(this).prepend('<img class="assigned" src="photo-fetch.php/tiny/' + encodeURIComponent(photo_base_name) + '"/>');
		  }
		});
	  $(".draggable").draggable({
		    // The helper and appendTo properties make the image appear above the li,
		    // but creates other problems:
		    //
		    // - The clone's top left corner, rather than the cursor
		    // location, becomes the draggable's position
  		    //
		    // - The clone doesn't get cleaned up upon dropping.
		    //
		    //helper: 'clone',
			//appendTo: 'body',
		    revert: 'invalid',
			scope: "assign",
			});

	  $(".photothumbs").droppable({
		    hoverClass: 'droppableHover',
			scope: 'undo',
			drop: function(event, ui) {
		  }
		});

	  $(".assigned").draggable({
		    revert: 'invalid',
			scope: 'undo',
			});
	});

</script>
<style type="text/css">
body {
  height: 100%;
  overflow: hidden;
}

ui-draggable-dragging {
  z-index: 200;
}

.draggable, .draggable * {
  z-index: 100;
}
</style>
</head>
<body>
<?php $banner_title = 'Racer Photos'; require('banner.inc'); ?>

<div class="block_buttons">
<form method="link">
  <input type="submit" value="Refresh"/>
</form>
</div>

<div class="thumblist">
<ul data-role="listview" class="ui-listview">
<?php
require_once('data.inc');
$rs = odbc_exec($conn, 'select racerid, lastname, firstname, imagefile, class'
				.' from registrationinfo'
				.' inner join classes'
				.' on registrationinfo.classid = classes.classid'
				.' order by lastname, firstname');

$racers_by_id = array();
$racers_by_photo = array();
$racers_by_name = array();
while (odbc_fetch_row($rs)) {
  $racer = array('firstname' => odbc_result($rs, 'firstname'),
				 'lastname' => odbc_result($rs, 'lastname'),
				 'class' => odbc_result($rs, 'class'),
				 'racerid' => odbc_result($rs, 'racerid'),
				 'imagefile' => odbc_result($rs, 'imagefile'));
  $racers_by_id[odbc_result($rs, 'RacerID')] = $racer;
  $racers_by_name[odbc_result($rs, 'lastname').', '.odbc_result($rs, 'firstname')] = $racer;
  echo '<li id="missing-'.$racer['racerid'].'"';
  
  if (odbc_result($rs, 'imagefile') !== "") {  // If there's an associated photo...
	$img_base = odbc_result($rs, 'imagefile');
	$img = pathinfo($img_base, PATHINFO_BASENAME);
	$racers_by_photo[$img] = $racer;
	if (array_search($img, $allfiles) === false) {
	  echo ' class="ui-li-static ui-li-has-thumb lost_photo"';
	} else {
	  // echo ' class="hidden"';
	  echo ' class="ui-li-static ui-li-has-thumb"';
	}
  } else {
	$img_base = '';
	echo ' class="ui-li-static ui-li-has-thumb droppable"';
  }
  echo '>';
  if ($img_base != '') {
	echo "\n".'<img class="assigned" src="photo-fetch.php/tiny/'.$img
	  .'" onclick="removeRacerPhoto('.$racer['racerid'].');"/>';
  }
  echo odbc_result($rs, 'FirstName').' '.odbc_result($rs, 'LastName');
  echo '<p>'.odbc_result($rs, 'class').'</p>';
  echo '</li>'."\n";
}

//foreach ($racers_by_photo as $photo => $racer) {
//}
?>
</ul>
</div>
<div class="photothumbs">
<?php
foreach ($allfiles as $imagefile) {
  echo '<div class="thumbnail'.(isset($racers_by_photo[$imagefile]) ? ' hidden' : '').'">';
  echo '<a href="photo-crop.php?name='.urlencode($imagefile).'">';
  echo '<img class="draggable" src="photo-fetch.php/thumb/';
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

  echo '</div>'."\n";
}
echo '<div class="thumbnail"><p>End of photos</p></div>';
echo '<div class="thumbnail"><p>End of photos</p></div>';
?>
</div>

<script type="text/javascript">

function changeRacerPhotoAjax(previous, racer, photo) {
  console.log('changeRacerPhotoAjax: previous racer = ' + previous
			  + ', new racer = ' + racer + ', photo = ' + photo);

   var xmlhttp = new XMLHttpRequest();
   xmlhttp.open("POST", g_checkin_action_url, /*async*/true);
   xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
   xmlhttp.onreadystatechange = readystate_handler;
   xmlhttp.send("action=photo&racer=" + racer
				+ "&previous=" + previous
				+ "&photo=" + encodeURIComponent("<?php echo addcslashes($photoThumbsDirectory.'\\', '\\\"'); ?>" + photo));
   ajax_add_request();
}

// TODO: This gives no visual feedback that the image got removed.
function removeRacerPhoto(previous) {
  changeRacerPhotoAjax(previous, 0, '');
}

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
  changeRacerPhotoAjax(previous, racer, photo);
}
</script>
</body>
<?php odbc_close($conn); ?>
</html>
