<?php @session_start();

require_once('inc/data.inc');
require_once('inc/banner.inc');
require_once('inc/authorize.inc');
require_once('inc/photo-config.inc');
require_permission(JUDGING_PERMISSION);
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Award Judging</title>
<?php require('inc/stylesheet.inc'); ?>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery-ui-1.10.4.min.js"></script>
<script type="text/javascript" src="js/jquery.ui.touch-punch.min.js"></script>
<script type="text/javascript" src="js/dashboard-ajax.js"></script>
<!-- script type="text/javascript" src="js/mobile-init.js"></script -->
<!-- script type="text/javascript" src="js/jquery.mobile-1.4.2.min.js"></script-->
<script type="text/javascript" src="js/modal.js"></script>
<script type="text/javascript" src="js/judging.js"></script>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<link rel="stylesheet" type="text/css" href="css/judging.css"/>
</head>
<body>
<?php
make_banner('Judging');

require_once('inc/standings.inc');

$racers = array();
$awards = array();

$sql = 'SELECT racerid, carnumber, lastname, firstname,'
      .' RegistrationInfo.classid, class, RegistrationInfo.rankid, rank, imagefile, carphoto'
      .' FROM '.inner_join('RegistrationInfo',
                           'Classes', 'Classes.classid = RegistrationInfo.classid',
                           'Ranks', 'Ranks.rankid = RegistrationInfo.rankid')
      .' WHERE passedinspection = 1 AND exclude = 0'
      .' ORDER BY carnumber';
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
                            'carphoto' => $rs['carphoto'],
							'awards' => array());
}

$n_den_trophies = read_raceinfo('n-den-trophies', 3);
$n_pack_trophies = read_raceinfo('n-pack-trophies', 3);
require_once('inc/ordinals.inc');

$speed_trophies = top_finishers_by_class($n_den_trophies);
foreach ($speed_trophies as $classid => $den_trophies) {
  for ($place = 0; $place < count($den_trophies); ++$place) {
	$racerid = $den_trophies[$place];
	$racers[$racerid]['awards'][] = ordinal(1 + $place).' in '.group_label_lc();
  }
}

$pack_trophies = top_finishers_overall($n_pack_trophies);
for ($place = 0; $place < count($pack_trophies); ++$place) {
  $racerid = $pack_trophies[$place];
  $racers[$racerid]['awards'][] = ordinal(1 + $place).' in '.supergroup_label_lc();
}

$stmt = $db->query('SELECT awardname, racerid'
  .' FROM Awards'
  .' ORDER BY sort');
foreach ($stmt as $rs) {
  $racerid = $rs['racerid'];
  if (isset($racers[$racerid]))
	$racers[$racerid]['awards'][] = $rs['awardname'];
}
?>
<div id="top_matter" class="block_buttons">
  <form method="link" action="awards-editor.php">
    <input type="submit" value="Edit Awards"/>
  </form>
</div>

<div id="awards">
  <ul data-rolex="listview">
  </ul>
</div>

<div id="racers">
  <?php

  $use_subgroups = read_raceinfo_boolean('use-subgroups');

  foreach ($racers as $racer) {
    echo "<div class='judging_racer' data-racerid='".$racer['racerid']."' onclick='show_racer_awards_modal($(this));'>\n";
    if ($racer['carphoto']) {
      echo "<img src='".car_photo_repository()->lookup(RENDER_JUDGING)->render_url($racer['carphoto'])."'/>";
    }
    echo "<div class='carno'>";
    echo $racer['carnumber'];
    echo "</div>";


    echo "<div class='racer_name'>".htmlspecialchars($racer['firstname'].' '.$racer['lastname'], ENT_QUOTES, 'UTF-8')."</div>";
    echo "<div class='group_name' data-classid='".$racer['classid']."'>"
        .htmlspecialchars($racer['class'], ENT_QUOTES, 'UTF-8')
    ."</div>";
    if ($use_subgroups) {
      echo "<div class='subgroup_name' data-rankid='".$racer['rankid']."'>"
          .htmlspecialchars($racer['rank'], ENT_QUOTES, 'UTF-8')
      ."</div>";
    }
    // These get hidden/unhidden by javascript code, based on award results
    echo "<img class='award_marker' src='img/award-ribbon-27x36.png'/>";
    echo "</div>\n";
  }
?>
</div>

<div id="racer_awards_modal" class="modal_dialog hidden block_buttons">
  <form id="racer_awards_form">
    <h3>Awards for <span id="racer_awards_recipient_carno"></span> <span id="racer_awards_recipient_name"></span></h3>

    <ul id="racer_awards">
    </ul>
   <input type="button" value="Close" data-enhanced="true"
          onclick='close_racer_awards_modal();'/>
  </form>
</div>

</body>
</html>
