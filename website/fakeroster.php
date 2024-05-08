<?php @session_start(); ?>
<?php
require_once('inc/data.inc');
require_once('inc/authorize.inc');
session_write_close();
require_once('inc/banner.inc');
require_once('inc/partitions.inc');
require_once('inc/plural.inc');

require_permission(CHECK_IN_RACERS_PERMISSION);

?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Build Fake Roster</title>
<?php require('inc/stylesheet.inc'); ?>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/mobile.js"></script>
<script type="text/javascript">
function make_the_fake_racers() {
  console.log('make_the_fake_racers');
  $.ajax("action.php",
         {type: 'POST',
          data: {action: 'racer.fake',
                 ngroups: Number($("#ngroups").val()),
                 racers_per_group: Number($("#racers_per_group").val()),
                 check_in: $("#check_in_all").is(':checked') ? 1 : 0},
          success: function(data) {
             console.log('success');
             // Jump to the check-in page
             window.location.href = "checkin.php";
          }
        });
  return false;
}

$(function() {
    $("#do_it").on('click', function() { make_the_fake_racers(); });
  });
</script>
<style type="text/css">
div input {
  font-size: 1em;
}
</style>
</head>
<body>
<?php make_banner('Build Fake Roster', 'setup.php'); ?>

<p>This page allows you to manufacture a roster of made-up racers.</p>

<p>A fake roster lets you experiment with the software without having to gather a list of your real racers.</p>

<p>The fake roster is randomly generated and reflects a typical race-by-den structure.  It includes fake racer and car images.</p>

<p>You can purge the fake roster data by <b>returning to the Set-Up page</b>.</p>

<div style="margin-left: 20px; font-size: 24px;">

Generate
<input id="ngroups" type="number" min="1" max="10" class="not-mobile" value="6"/>
<?php echo plural(group_label()); ?>
<br/>
of approximately
<input id="racers_per_group" type="number" min="2" max="20" class="not-mobile" value="5"/>
fake racers each.
<br/>

<p>
<input id="check_in_all" type="checkbox" checked="checked"/>
<label for="check_in_all">Fake racers have passed inspection</label>
</p>
<br/>

<div class="block_buttons" style="width: 350px;">
<input id="do_it" type="button" value="Make Fake Racers"/>
</div>

</div>

<div id="attributions" style="position: fixed; bottom: 0;">
Some fake racer images came from one or more of these sources: <br/>
  <a href="https://www.vecteezy.com/free-vector/artwork">Artwork Vectors by Vecteezy</a> <br/>
  <a href="https://www.vecteezy.com/free-vector/portrait">Portrait Vectors by Vecteezy</a> <br/>
  <a href="https://www.vecteezy.com/free-vector/doodle">Doodle Vectors by Vecteezy</a> <br/>
  <a href="https://www.vecteezy.com/free-vector/family">Family Vectors by Vecteezy</a> <br/>

<a href="https://www.vecteezy.com/free-vector/race-car">Race Car Vectors by Vecteezy</a> <br/>
<a href="https://www.vecteezy.com/free-vector/race-car">Race Car Vectors by Vecteezy</a> <br/>
<a href="https://www.vecteezy.com/free-vector/car">Car Vectors by Vecteezy</a> <br/>
<a href="https://www.vecteezy.com/free-vector/cardboard-box">Cardboard Box Vectors by Vecteezy</a>

</div>

</body>
</html>
