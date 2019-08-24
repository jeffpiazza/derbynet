<!DOCTYPE html>
<html lang="en" itemscope itemtype="http://schema.org/Product">
<head>
<title>Fullscreen Kiosk</title>
<?php require('inc/stylesheet.inc'); ?>
<style type="text/css">

.full-window {
  position: absolute;
  top: 0;
  height: 100%;
  left: 0;
  width: 100%;
}

#setup {
margin-top: 100px;
}
</style>
<?php
if (isset($_SERVER['SERVER_NAME'])) {
  $server = $_SERVER['SERVER_NAME'];
} else if (isset($_SERVER['SERVER_ADDR'])) {
  $server = $_SERVER['SERVER_ADDR'];
} else if (isset($_SERVER['LOCAL_ADDR'])) {
  $server = $_SERVER['LOCAL_ADDR'];
} else if (isset($_SERVER['HTTP_HOST'])) {
  $server = $_SERVER['HTTP_HOST'];
} else {
  $addrs = gethostbynamel(gethostname());
  if (count($addrs) > 0) {
    $server = $addrs[0];
  } else {
    $server = ' unknown server name! ';
  }
}

if (isset($_SERVER['REQUEST_URI'])) {
  $path = $_SERVER['REQUEST_URI'];
} else if (isset($_SERVER['PHP_SELF'])) {
  $path = $_SERVER['PHP_SELF'];
} else {
  $path = $_SERVER['SCRIPT_NAME'];
}

$url = $server . $path;

$last = strrpos($url, '/');
if ($last === false) {
  $last = -1;
}

// Don't force http !
$kiosk_url = '//'.substr($url, 0, $last + 1).'kiosk.php';
?>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/screenfull.min.js"></script>

<script type="text/javascript">
function on_proceed() {
  $("#interior").attr('src', $("#inner_url").val());
  $("#setup").addClass('hidden');
  $("#interior").removeClass('hidden');

  if (screenfull.enabled) {
    screenfull.request();
  }
}
</script>
</head>

<body>

<iframe id="interior" class="hidden full-window">
</iframe>

<div id="setup" class ="full-window block_buttons">

  <label for="inner_url">URL:</label>
  <input type="text" name="inner_url" id="inner_url" size="100"
         value="<?php echo htmlspecialchars($kiosk_url, ENT_QUOTES, 'UTF-8'); ?>"/>
  <input type="button" data-enhanced="true" value="Proceed"
         onclick="on_proceed();"/>
</div>
</body>
</html>