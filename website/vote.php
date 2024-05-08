<?php @session_start();

require_once('inc/data.inc');
require_once('inc/authorize.inc');
session_write_close();
require_once('inc/banner.inc');
require_once('inc/photo-config.inc');
require_once('inc/awards.inc');
require_once('inc/voterid.inc');
require_once('inc/standings.inc');
require_once('inc/schema_version.inc');

$is_open = read_raceinfo('balloting', 'closed') == 'open';

?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Award Ballot</title>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/ajax-setup.js"></script>
<script type="text/javascript" src="js/jquery-ui.min.js"></script>
<script type="text/javascript" src="js/jquery.ui.touch-punch.min.js"></script>
<script type="text/javascript" src="js/modal.js"></script>
<script type="text/javascript" src="js/vote.js"></script>
<script type="text/javascript">
var g_ballot;
var g_awardid;
var g_racerid;
<?php if ($is_open) { ?>
  $(function() { get_ballot(); });
<?php } ?>
</script>
<?php require('inc/stylesheet.inc'); ?>
<link rel="stylesheet" type="text/css" href="css/vote.css"/>
</head>
<body>
<?php
make_banner('Ballot');

$order = '';
if (isset($_GET['order']))
  $order = $_GET['order'];  // Values are: name, class, car
if (!$order)
    $order = 'car';

function link_for_ordering($key, $text) {
  global $order;
  echo "<a ";
  if ($order == $key) {
    echo 'class="current_sort"';
  }
  echo " href='ballot.php?order=".$key."'>";
  echo $text;
  echo "</a>";
}

$racers = array();

$sql = 'SELECT racerid, carnumber, lastname, firstname,'
      .' RegistrationInfo.classid, class, RegistrationInfo.rankid, rank, imagefile,'
      .' '.(schema_version() < 2 ? "class" : "Classes.sortorder").' AS class_sort, '
      .(schema_version() < 2 ? '\'\' as ' : '').' carphoto'
      .' FROM '.inner_join('RegistrationInfo',
                           'Classes', 'Classes.classid = RegistrationInfo.classid',
                           'Ranks', 'Ranks.rankid = RegistrationInfo.rankid')
      .' WHERE passedinspection = 1 AND exclude = 0'
      .' ORDER BY '
      .($order == 'car' ? 'carnumber, lastname, firstname' :
        ($order == 'class'  ? 'class_sort, lastname, firstname' :
         'lastname, firstname'));

foreach ($db->query($sql) as $rs) {
  $racerid = $rs['racerid'];
  $racers[$racerid] = array('racerid' => $racerid,
							'carnumber' => $rs['carnumber'],
							'lastname' => $rs['lastname'],
							'firstname' => $rs['firstname'],
                            'classid' => $rs['classid'],
							'class' => $rs['class'],
                            'rankid' => $rs['rankid'],
                            'rank' => $rs['rank'],
                            'imagefile' => $rs['imagefile'],
                            'carphoto' => $rs['carphoto']);
}

?>

<div id="awards">
  <div <?php if ($is_open) echo 'class="hidden"'; ?>>
    <h3>Balloting is currently closed.</h3>
  </div>
  <div id='no-awards' class='hidden'>
    <h3>There are no awards available for voting.</h3>
  </div>
  <div class='please-click hidden'>
    Please click on an award to vote.
  </div>
  <?php
  // This enumerates all awards, not just those that are ballotable.
  $awards = all_awards(false);
  mark_award_eligibility($awards);
  foreach ($awards as $award) {
    echo "<div class='award hidden' data-awardid='$award[awardid]'";
    if ($award['classid'] != 0) {
      echo " data-classid='$award[classid]'";
    }
    if ($award['rankid'] != 0) {
      echo " data-rankid='$award[rankid]'";
    }
    echo " data-eligible-classids='".implode(',', $award['eligible-classids'])."'";
    echo " data-eligible-rankids='".implode(',', $award['eligible-rankids'])."'";
    echo " onclick='click_one_award($(this));'>";
    echo "<p class='award-name'>".htmlspecialchars($award['awardname'], ENT_QUOTES, 'UTF-8')."</p>";
    echo "<p class='please-vote-for'>Please vote for no more than <span class='please-vote-count'>UNSET</span>.</p>";
    echo "</div>\n";
  }
  ?>
</div>

<div id="racers_modal" class="modal_dialog hidden">
  <div id="racers_modal_headline" class="block_buttons">
    <input type="button" value="Close" data-enhanced="true"
      onclick='close_modal("#racers_modal");'/>
    <span>Please choose up to <span id="racer_view_max_votes"></span> for <span id="racer_view_award_name"></span></span>
    <span id="selected_racers">
    </span>
  </div>
<div id="racers">
<?php

  $use_subgroups = read_raceinfo_boolean('use-subgroups');

  foreach ($racers as $racer) {
    // Javascript depends on each div.ballot_racer having data-racerid, data-img
    // for the full-size image, an interior img element for thumbnail, and an
    // interior div.carno with the car number.
    echo "<div class='ballot_racer'"
        ." data-racerid='$racer[racerid]'"
        ." data-classid='$racer[classid]'"
        ." data-rankid='$racer[rankid]'"
        ." data-img='".car_photo_repository()->url_for_racer($racer, RENDER_WORKING)."'"
        ." onclick='show_racer_view_modal($(this));'>\n";
    if ($racer['carphoto']) {
      echo "<img src='".car_photo_repository()->lookup(RENDER_JUDGING)->render_url($racer['carphoto'])."'/>";
    }
    echo "<div class='carno'>";
    echo $racer['carnumber'];
    echo "</div>";


    // echo "<div class='racer_name'>".htmlspecialchars($racer['firstname'].' '.$racer['lastname'], ENT_QUOTES, 'UTF-8')."</div>";
    echo "<div class='group_name'>"
        .htmlspecialchars($racer['class'], ENT_QUOTES, 'UTF-8')
    ."</div>";
    if ($use_subgroups) {
      echo "<div class='subgroup_name'>"
          .htmlspecialchars($racer['rank'], ENT_QUOTES, 'UTF-8')
      ."</div>";
    }

    echo "</div>\n";
  }
?>
</div><!-- racers -->
</div><!-- racers_modal -->

<div id='racer_view_modal' class='modal_dialog hidden block_buttons'>
  <div id='racer_view_check' onclick='toggle_vote($(this));'>
  Vote:
    <img src="img/checkbox-with-check.png"/>
  </div>
  <div id='racer_view_carnumber'></div>

  <img id='racer_view_photo'/>
  <br/>
  <p id="full-ballot">You already have <span id="full-ballot-max">UNSET</span> racer(s) chosen.</p>
  <input type="button" value="Close" data-enhanced="true"
    onclick='close_secondary_modal("#racer_view_modal");'/>
</div>

<div id='password_modal' class='modal_dialog hidden block_buttons'>
  <form>
    <h3>Please enter the ballot password</h3>
    <input id="password_input" type='password'/>
    <p id="wrong-password" class="hidden">The password you entered is incorrect.  Please try again.</p>
    <input type='submit'/>
  </form>
</div>

</body>
</html>
