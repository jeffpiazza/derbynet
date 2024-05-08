<?php session_start();
require_once('inc/data.inc');
require_once('inc/authorize.inc');
session_write_close();
require_once('inc/banner.inc');
require_once('inc/schema_version.inc');
require_once('inc/partitions.inc');

require_permission(ASSIGN_RACER_IMAGE_PERMISSION);

require_once('inc/photo-config.inc');

$repo = isset($_GET['repo']) ? $_GET['repo'] : 'head';
$photo_repository = photo_repository($repo);
$other_repo =  $repo == 'car' ? 'head' : 'car';


$order = '';
if (isset($_GET['order']) && in_array($_GET['order'], ['name', 'class', 'car']))
  $order = $_GET['order'];
if (!$order)
    $order = 'name';

function link_for_ordering($key, $text) {
  global $order, $photo_repository;
  echo "<div class='sort_div'>";
  echo "<a class='sort_button button_link";
  if ($order == $key) {
    echo ' current_sort';
  }
  echo "' href='photo-thumbs.php?repo=".$photo_repository->name()."&amp;order=".$key."'>";
  //echo "<span class='sort_by'>Sort by</span>";
  //echo "<br/>";
  echo $text;
  echo "</a>";
  echo "</div>";
}

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

  asort($files);

  return $files;
}

$allfiles = scan_directory($photo_repository->directory(),
						   "/(jpg|jpeg|png|gif|bmp|svg)\$/i");

// Returns a javascript expression, suitable for onclick, to perform cropping of a particular photo.
function photo_crop_expression($basename) {
  global $photo_repository;
  return htmlspecialchars('showPhotoCropModal(this, "'.$photo_repository->name().'", "'.$basename.'", '.time().')',
                          ENT_QUOTES, 'UTF-8');
}

// TODO: line-height?  "End of photos" text aligns with thumbnail image bottom.
// *** Both div's are overhanging the bottom by the amount taken up by the banner and refresh button!
// *** height=100% could be at issue.
?>
<!DOCTYPE HTML>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Assign Racer Photos</title>
<meta name="viewport" content="width=device-width, initial-scale=1"/>
<link rel="stylesheet" type="text/css" href="css/jquery-ui.min.css"/>
<link rel="stylesheet" type="text/css" href="css/mobile.css"/>
<link rel="stylesheet" type="text/css" href="css/jquery.Jcrop.min.css"/>
<link rel="stylesheet" type="text/css" href="css/dropzone.min.css"/>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/photo-thumbs.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/ajax-setup.js"></script>
<script type="text/javascript" src="js/dashboard-ajax.js"></script>
<script type="text/javascript" src="js/jquery-ui.min.js"></script>
<script type="text/javascript" src="js/jquery.ui.touch-punch.min.js"></script>
<script type="text/javascript" src="js/dashboard-ajax.js"></script>
<script type="text/javascript" src="js/modal.js"></script>
<script type="text/javascript" src="js/jquery.Jcrop.min.js"></script>
<script type="text/javascript" src="js/dropzone.min.js"></script>
<script type="text/javascript">
var g_photo_repo_name = '<?php echo $photo_repository->name(); ?>';
<?php if (isset($_GET['racerid']) && is_numeric($_GET['racerid'])) { ?>
  $(function() { scroll_to_racerid(<?php echo $_GET['racerid']; ?>); });
<?php } ?>
</script>
<script type="text/javascript" src="js/photo-thumbs.js"></script>
</head>
<body>
<?php make_banner(($repo == 'head' ? 'Racer' : 'Car').' Photos',
                  isset($_GET['back']) ? $_GET['back'] : 'index.php'); ?>

<div class="block_buttons">

<div id="sort_controls">
<?php link_for_ordering('name', "Name"); ?>
<?php link_for_ordering('class', group_label()); ?>
<?php link_for_ordering('car', "Car#"); ?>.

</div>

<?php if (!is_writable($photo_repository->directory())) { ?>

  <div id="upload_target">
    <div class="dz-message">
      <span>Photo directory is not writable; check <a href="settings.php">settings.</a></span>
    </div>
  </div>

<?php } else { ?>

  <form id="upload_target" action="action.php" class="dropzone">
    <div class="fallback">
      <input type="file" name="photo" value="Upload Files"/>
    </div>
    <input type="hidden" name="action" value="photo.upload"/>
    <input type="hidden" name="repo" value="<?php echo $photo_repository->name(); ?>"/>
    <input type="hidden" name="MAX_FILE_SIZE" value="30000000" />
  </form>

<?php } ?>

   <div id="center_buttons">
    <?php
        echo "<a class='button_link' id='refresh-button' onclick='window.location.reload();'>Refresh</a>";
        $url = "photo-thumbs.php?repo=$other_repo&amp;order=$order";
        if (isset($_GET['racerid']) && is_numeric($_GET['racerid'])) {
          $url .= "&amp;racerid=$_GET[racerid]";
        }
        if (isset($_GET['back'])) {
          $url .= "&amp;back=$_GET[back]";
        }
        echo "<a id='other-button' class='button_link' href='$url'>";
        echo   $other_repo == 'head' ? 'Racers' : 'Cars';
        echo "</a>";
    ?>
    </div>
  </div>

<div class="body-wrapper">

<div class="thumblist">
<ul class="mlistview has-thumbs">
<?php
require_once('inc/data.inc');

$use_groups = use_groups();

$racers_by_photo = array();
$stmt = $db->query('SELECT racerid, lastname, firstname, '.$photo_repository->column_name().','
                   .' carnumber, class,'
                   .' '.(schema_version() < 2 ? "class" : "Classes.sortorder").' AS class_sort '
				   .' FROM RegistrationInfo'
				   .' INNER JOIN Classes'
				   .' ON RegistrationInfo.classid = Classes.classid'
                   .' ORDER BY '
                   .($order == 'car' ? 'carnumber, lastname, firstname' :
                     ($order == 'class'  ? 'class_sort, lastname, firstname' :
                      'lastname, firstname')));

foreach ($stmt as $rs) {
  $raw_imagefile = $rs[$photo_repository->column_name()];
  $racer = array('firstname' => $rs['firstname'],
				 'lastname' => $rs['lastname'],
				 'class' => $rs['class'],
				 'racerid' => $rs['racerid'],
				 'imagefile' => $raw_imagefile);

  // CSS classes control drag and drop behavior
  $css_classes = '';
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
      .' onclick="'.photo_crop_expression($image_filename).'"'
      .' src="'.$photo_repository->lookup(RENDER_LISTVIEW)->render_url($image_filename).'"/>';
  }
  echo htmlspecialchars($rs['firstname'].' '.$rs['lastname'], ENT_QUOTES, 'UTF-8');
  if ($use_groups) {
    echo '<p><strong>'.$rs['carnumber'].':</strong> '.htmlspecialchars($rs['class'], ENT_QUOTES, 'UTF-8').'</p>';
  } else {
    echo '<p><strong>'.$rs['carnumber'].'</strong></p>';
  }
  echo '</li>'."\n";
}

?>
</ul>

</div>

<div class="photothumbs">
<?php
foreach ($allfiles as $imagefile) {
  echo '<div class="thumbnail'.(isset($racers_by_photo[$imagefile]) ? ' hidden' : '').'">';
  echo '<img class="unassigned-photo"'
      .' data-image-filename="'.htmlspecialchars($imagefile, ENT_QUOTES, 'UTF-8').'"'
      .' onclick="'.photo_crop_expression($imagefile).'"'
      .' src="'.$photo_repository->lookup(RENDER_THUMBNAIL)->render_url($imagefile).'"/>';
  echo '</div>'."\n";
}

if (empty($allfiles)) {
  $trouble_first = '<h2 class="trouble"><img src="img/status/trouble.png"/>';
  $dir_name = htmlspecialchars($photo_repository->directory(), ENT_QUOTES, 'UTF-8');
  $trouble_last = "</h2>\n";
  if (!file_exists($photo_repository->directory())) {
    echo $trouble_first.'Directory '.$dir_name.' does not exist.'.$trouble_last;
  } else if (!is_dir($photo_repository->directory())) {
    echo $trouble_first.'Directory path '.$dir_name.' exists but is not a directory.'.$trouble_last;
  } else if (!is_readable($photo_repository->directory())) {
    echo $trouble_first.'Directory '.$dir_name.' cannot be read.'.$trouble_last;
  } else if (!is_writable($photo_repository->directory())) {
    echo $trouble_first.'Directory '.$dir_name.' is not writable.'.$trouble_last;
  } else {
    echo '<h2>There are no photos in the photo directory yet.</h2>';
  }
}
?>
</div>

</div>

<div id="photo_crop_modal" class="modal_dialog hidden block_buttons">
<p id='photo_basename'></p>
<div id="work_image"></div>

<p id="crop_instructions">Indicate new crop boundary, <i>then</i> press Crop.</p>
<input type="button" value="Crop" onclick="cropPhoto(); return false;"/>
<input type="button" value="Rotate Right" onclick="rotatePhoto(-90); return false;"/>
<input type="button" value="Rotate Left" onclick="rotatePhoto(90); return false;"/>
<input type="button" value="Cancel" onclick="close_modal('#photo_crop_modal');"/>

<input type="button" value="Delete"
    class="delete_button"
    onclick="on_delete_photo_button(); return false;"/>
</div>

<div id="ajax_working" class="hidden">
  <span id="ajax_num_requests">0</span> request(s) pending.
</div>


<div id="delete_confirmation_modal" class="modal_dialog block_buttons hidden">
  <form>
    <p>Are you sure you want to delete this photo?</p>

    <input type="submit" value="Delete Photo"/>

    <p>&nbsp;</p>
    <input type="button" value="Cancel"
      onclick='close_secondary_modal("#delete_confirmation_modal");'/>
  </form>
</div>

</body>
</html>
