<?php
@session_start();
$imagename = $_GET['name'];
require_once('inc/photo-config.inc');
?>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Edit Photo</title>
<?php require('inc/stylesheet.inc'); ?>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery.Jcrop.min.js"></script>
<link rel="stylesheet" type="text/css" href="css/jquery.Jcrop.min.css"/>
</head>
<body>
<?php
echo '<img src="photo-fetch.php/work/';
$work_image = $photoWorkDirectory.DIRECTORY_SEPARATOR.$imagename;
if (file_exists($work_image)) {
  echo @filemtime($work_image).'/';
}
echo urlencode($imagename).'"/ id="target">';
?>
<script type="text/javascript">
jQuery(function($) {
  $('#target').Jcrop({
	aspectRatio: <?php echo $thumbWidth; ?>/<?php echo $thumbHeight; ?>,
	onSelect: updateForm,
	onChange: updateForm
  });
});

function updateForm(c) {
  $('#top').val(c.y);
  $('#left').val(c.x);
  $('#bottom').val(c.y2);
  $('#right').val(c.x2);
  $('#original_height').val($('#target').height());
  $('#original_width').val($('#target').width());
}

</script>
<div class="block_buttons">
<form method="post" action="photo-crop-action.php">
  <input type="hidden" id="image_name" name="image_name"
         value="<?php echo htmlspecialchars($imagename, ENT_QUOTES, 'UTF-8'); ?>"/>
  <input type="hidden" id="original_height" name="original_height" value=""/>
  <input type="hidden" id="original_width" name="original_width" value=""/>
  <input type="hidden" id="top" name="top" value=""/>
  <input type="hidden" id="left" name="left" value=""/>
  <input type="hidden" id="right" name="right" value=""/>
  <input type="hidden" id="bottom" name="bottom" value=""/>
  <input type="submit" value="Crop"/>
</form>
<form method="post" action="photo-crop-action.php">
  <input type="hidden" id="image_name" name="image_name"
         value="<?php echo htmlspecialchars($imagename, ENT_QUOTES, 'UTF-8'); ?>"/>
  <input type="hidden" name="rotation" value="90"/>
  <input type="submit" value="Rotate Right"/>
</form>
<form method="post" action="photo-crop-action.php">
  <input type="hidden" id="image_name" name="image_name"
         value="<?php echo htmlspecialchars($imagename, ENT_QUOTES, 'UTF-8'); ?>"/>
  <input type="hidden" name="rotation" value="-90"/>
  <input type="submit" value="Rotate Left"/>
</form>
</div>
</body>
</html>
