<!DOCTYPE html>
<html lang="en" itemscope itemtype="http://schema.org/Product">
<head>
<title>Fullscreen Kiosk</title>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/screenfull.min.js"></script>
</head>

<body>

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
$kiosk_url = 'http://'.substr($url, 0, $last + 1).'kiosk.php';
?>

<form onsubmit="go_fullscreen(); return false;" style="margin-top: 100px; margin-left: auto; margin-right: auto;">
  <input type="text" size="100" id="url" value="<?php echo htmlspecialchars($kiosk_url, ENT_QUOTES, 'UTF-8'); ?>"/>
<input type="submit"/>
</form>

<script type="text/javascript">
 function go_fullscreen() {
   if (screenfull.enabled) {
     screenfull.request();
   }

   // We create an iframe and fill the window with it
   var iframe = document.createElement('iframe');
   iframe.setAttribute('id', 'external-iframe');
   iframe.setAttribute('src', $('#url').val());
   iframe.setAttribute('frameborder', 'no');

   iframe.style.position = 'absolute';
   iframe.style.top = '0';
   iframe.style.right = '0';
   iframe.style.bottom = '0';
   iframe.style.left = '0';
   iframe.style.width = '100%';
   iframe.style.height = '100%';

   $('body form').remove();

   $('body').prepend(iframe);
   document.body.style.overflow = 'hidden';
 }

// TODO $(function() { setTimeout(function() { go_fullscreen(); }, 1000); });
</script>
</body>
</html>