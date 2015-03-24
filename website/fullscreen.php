<!doctype html>
<html lang="en" itemscope itemtype="http://schema.org/Product">
<head>
<title>Fullscreen Kiosk</title>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/screenfull.min.js"></script>
</head>

<body>

<?php
$last = strrpos($_SERVER['REQUEST_URI'], '/');
if ($last === false) {
  $last = -1;
}
$url = $_SERVER['REQUEST_SCHEME'].'://'.$_SERVER['SERVER_NAME'].substr($_SERVER['REQUEST_URI'], 0, $last + 1).'kiosk.php';
?>

<form onsubmit="go_fullscreen(); return false;" style="margin-top: 100px; margin-left: auto; margin-right: auto;">
  <input type="text" size="100" id="url" value="<?php echo $url; ?>"/>
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
   // Setting the background color to black MAY help avoid a white screen
   // appearing when the kiosk page reloads.  Not sure it has any effect, but at
   // least it shouldn't hurt.
   $('body').css({'background-color': 'black'});

   $('body').prepend(iframe);
   document.body.style.overflow = 'hidden';
 }

// TODO $(function() { setTimeout(function() { go_fullscreen(); }, 1000); });
</script>
</body>
</html>