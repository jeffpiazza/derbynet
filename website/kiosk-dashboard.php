<?php session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_permission(PRESENT_AWARDS_PERMISSION);
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Kiosk Dashboard</title>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery-ui-1.10.4.min.js"></script>
<link rel="stylesheet" type="text/css" href="css/kiosk-dashboard.css"/>
<script type="text/javascript">
// We're using jQuery Mobile for its nice mobile/tablet-friendly UI
// elements.  By default, jQM also wants to hijack page loading
// functionality, and perform page loads via ajax, a "service" we
// don't really want.  Fortunately, it's possible to turn that stuff
// off.
$(document).bind("mobileinit", function() {
                                   $.extend($.mobile, {
                                         ajaxEnabled: false
                                         });
                                 });
</script>
<script type="text/javascript" src="js/jquery.mobile-1.4.2.min.js"></script>
<script type="text/javascript" src="js/dashboard-ajax.js"></script>
<script type="text/javascript" src="js/modal.js"></script>
<script type="text/javascript" src="js/kiosk-management.js"></script>
<script type="text/javascript" src="js/kiosk-dashboard.js"></script>
</head>
<body>
<?php $banner_title = 'Kiosk Dashboard'; require('inc/banner.inc'); ?>
<?php require_once('inc/ajax-failure.inc'); ?>

<div class="hidden standings-control control_group block_buttons">

<div class="round-select">
<h3>Display standings for:</h3>
<?php
$current = read_raceinfo('standings-message');
$current_roundid = explode('-', $current)[0];
?>

<select>
   <?php
      require_once('inc/standings.inc');

      $sel = ' selected="selected"';
      if ($current == '') {
        echo '<option '.$sel.' disabled="1">Please choose what standings to display</option>';
      }
      echo '<option data-roundid=""'.(($current != '' && $current_roundid == '') ? $sel : '').'>'
           .supergroup_label()
           .'</option>';
      foreach (standings_round_names() as $round) {
        echo '<option data-roundid="'.$round['roundid'].'"'.($current_roundid == $round['roundid'] ? $sel : '').'>'
               .htmlspecialchars($round['name'], ENT_QUOTES, 'UTF-8')
               .'</option>'."\n";
      }
  ?>
</select>
</div>
<div class="reveal block_buttons">
<input type="button" data-enhanced="true" value="Reveal 1" onclick="handle_reveal1()"/><br/>
<input type="button" data-enhanced="true" value="Reveal All" onclick="handle_reveal_all()"/><br/>
</div>
</div>

<div id="kiosk_control_group" class="kiosk_control_group">
</div>


<div id='kiosk_modal' class="modal_dialog hidden block_buttons">
  <form>
    <label for="kiosk_name_field">Name for kiosk:</label>
    <input type="text" id="kiosk_name_field"/>
    <input type="submit" data-enhanced="true" value="Assign"/>
    <input type="button" data-enhanced="true" value="Cancel"
      onclick='close_modal("#kiosk_modal");'/>
  </form>
</div>

</body>
</html>