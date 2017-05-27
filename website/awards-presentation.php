<?php @session_start();
// Controls the "current award" kiosk display
require_once('inc/data.inc');
require_once('inc/banner.inc');
require_once('inc/authorize.inc');
require_once('inc/schema_version.inc');
require_permission(PRESENT_AWARDS_PERMISSION);
?><!DOCTYPE html>
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
<title>Awards Presentation Dashboard</title><?php require('inc/stylesheet.inc'); ?>
<script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/jquery-ui-1.10.4.min.js"></script>
<script type="text/javascript" src="js/dashboard-ajax.js"></script>
<script type="text/javascript" src="js/mobile-init.js"></script>
<script type="text/javascript" src="js/jquery.mobile-1.4.2.min.js"></script>
<script type="text/javascript" src="js/awards-presentation.js"></script>
<?php
    $nkiosks = read_single_value('SELECT COUNT(*) FROM Kiosks'
                                 .' WHERE page LIKE \'%award%present%\'', array());
    if ($nkiosks == 0) {
      echo '<script type="text/javascript">'."\n";
      echo '$(window).load(function() {'."\n";
      echo '  setTimeout(function() {'."\n";
      echo '  alert("NOTE: There are NO kiosks ready for award presentation."+'."\n";
      echo '        "  Selections on this dashboard won\'t have any observable effect.");'."\n";
      echo '}, 500); });'."\n";
      echo '</script>'."\n";
    }
?>
<link rel="stylesheet" type="text/css" href="css/jquery.mobile-1.4.2.css"/>
<link rel="stylesheet" type="text/css" href="css/awards-presentation.css"/>
</head>
<body>
<?php make_banner('Awards Presentation');

require_once('inc/standings.inc');
require_once('inc/ordinals.inc');
require_once('inc/awards.inc');

$use_subgroups = read_raceinfo_boolean('use-subgroups');
$n_den_trophies = read_raceinfo('n-den-trophies', 3);
$n_pack_trophies = read_raceinfo('n-pack-trophies', 3);

list($classes, $classseq, $ranks, $rankseq) = classes_and_ranks();

// $bins to a bin.

// A bin_key is a string:
//  'p' (overall group)
//  'c' + classid
//  'r' + rankid
//
// A bin is an array with keys:
//  name   => bin name (string)
//  awards => array of awards, each award being an array of:
//    bin_key?
//    classid
//    rankid
//    awardkey => string for presentation
//    awardname
//    awardtype
//    awardtypeid
//    sort
//    firstname
//    lastname
//    carnumber

// GPRM views classes and ranks as a hierarchy.  Selecting "none" shows all
// awards (any class/rank) in the category, selecting a class shows all the
// awards in the class (any rank), selecting a rank shows just the awards in
// that rank.

// Not sure what's going on with Speed Trophy awardtype in GPRM; it's not
// offered as a choice for explicit awards.  "Speed Standings" is a choice for
// the Awards page, though.

// Future TODO: Allow the race coordinator to map speed results to awards, e.g.,
// "top 3 finishers get Speed Trophy awards"?

// TODO If using subgroups, capture speed standings by rank, in addition to
// capturing by class

// TODO Provide for auto-generating participation awards

// TODO Provide a page for entering/sorting awards.  Also award import.

// TODO Include award export?

$awards = array();
// $speed_awards_in_bin maps bin_key to a count of speed awards in that bin
$speed_awards_in_bin = array();

function bin_key($classid, $rankid) {
  if (!isset($classid)) {
    return 'p';
  } else if (!isset($rankid)) {
    return 'c'.$classid;
  } else {
    return 'r'.$rankid;
  }
}

function add_speed_award(&$row, $classid, $rankid, $limit, $label) {
  global $awards, $speed_awards_in_bin;
  $key = bin_key(@$classid, @$rankid);
  if (!isset($speed_awards_in_bin[$key])) {
    $speed_awards_in_bin[$key] = 0;
  }
  $place = ++$speed_awards_in_bin[$key];
  if ($place <= $limit) {
    $awards[] = array('bin_key' => $key,
                      'classid' => @$classid,
                      'rankid' => @$rankid,
                      'awardkey' => 'speed-'.$place.(isset($classid) ? '-'.$classid : ''),
                      'awardname' => nth_fastest($place, $label),
                      // TODO 'Speed Trophy' and 5 should be user-selectable, not hard wired like this.
                      'awardtype' => 'Speed Trophy',
                      'awardtypeid' => 5,
                      // TODO Sort order for speed awards needs user control?
                      'sort' => $place,
                      'firstname' => $row['firstname'],
                      'lastname' => $row['lastname'],
                      'carnumber' => $row['carnumber']);
  }
}

// Collect speed awards
foreach (final_standings() as $row) {
  // Can be BOTH for_group and for_supergroup
  if ($row['for_supergroup']) {
    add_speed_award($row, null, null, $n_pack_trophies, supergroup_label());
  }
  if ($row['for_group']) {
    add_speed_award($row, @$row['classid'], null, $n_den_trophies, $classes[$row['classid']]['class']);
  }
  // TODO Speed awards by rank, as well as by class?
}

// TODO Break ties for award sorting according to class and rank ordering
foreach ($db->query('SELECT awardid, awardname, awardtype,'
                    .' Awards.awardtypeid, Awards.classid, Awards.rankid, sort,'
                    .' firstname, lastname, carnumber'
                    .' FROM '.inner_join('Awards', 'AwardTypes',
                                         'Awards.awardtypeid = AwardTypes.awardtypeid',
                                         'RegistrationInfo',
                                         'Awards.racerid = RegistrationInfo.racerid')
                    .' ORDER BY sort, lastname, firstname') as $row) {
  $awards[] =
      array('bin_key' => bin_key(@$row['classid'], @$row['rankid']),
            'classid' => @$row['classid'],
            'rankid' => @$row['rankid'],
            'awardkey' => 'award-'.$row['awardid'],
            'awardname' => $row['awardname'],
            'awardtype' => $row['awardtype'],
            'awardtypeid' => $row['awardtypeid'],
            'sort' => $row['sort'],
            'firstname' => $row['firstname'],
            'lastname' => $row['lastname'],
            'carnumber' => $row['carnumber']);
}

function compare_by_sort(&$lhs, &$rhs) {
  return $lhs['sort'] == $rhs['sort'] ? 0 :
      $lhs['sort'] < $rhs['sort'] ? -1 : 1;
}

usort($awards, 'compare_by_sort');
?>

<div class="block_buttons">

<div class="center-select">
    <select id="awardtype-select">
        <option selected="Selected">All Awards</option>
        <?php
        foreach ($db->query('SELECT awardtypeid, awardtype FROM AwardTypes ORDER BY awardtype') as $atype) {
          echo '<option data-awardtypeid="'.$atype['awardtypeid'].'">'
              .htmlspecialchars($atype['awardtype'], ENT_QUOTES, 'UTF-8')
              .'</option>'."\n";
        }
        ?>
    </select>
</div>

<div class="center-select">
    <select id="group-select">
      <option selected="Selected"><?php echo supergroup_label(); ?></option>
        <?php
        $classid = -1;
        foreach ($rankseq as $rankid) {
          $rank = $ranks[$rankid];
          if ($rank['classid'] != $classid) {
              $classid = $rank['classid'];
              echo '<option data-classid="'.$classid.'">'
               .htmlspecialchars($rank['class'], ENT_QUOTES, 'UTF-8')
               .'</option>'."\n";
          }
          if ($use_subgroups) {
            echo '<option data-classid="'.$classid.'" data-rankid="'.$rank['rankid'].'">'
                 .htmlspecialchars($rank['rank'], ENT_QUOTES, 'UTF-8')
                 .'</option>'."\n";
          }
        }
        ?>
    </select>
</div>

<div class="listview">
<ul data-role="listview" class="ui-listview">
<?php

foreach ($awards as &$row) {
   $classid = isset($row['classid']) ? $row['classid'] : 0;
   $rankid = (isset($row['rankid']) && $use_subgroups) ? $row['rankid'] : 0;
    echo '<li class="ui-btn ui-btn-icon-right ui-icon-carat-r"'
        .' onclick="on_choose_award(this);"'
        .' data-awardkey="'.$row['awardkey'].'"'
        .' data-awardtypeid="'.$row['awardtypeid'].'"'
        .' data-classid="'.$classid.'"'
        .' data-rankid="'.$rankid.'"'
        .' data-awardname="'.htmlspecialchars($row['awardname'], ENT_QUOTES, 'UTF-8').'"'
        .' data-recipient="'.htmlspecialchars($row['firstname'].' '.$row['lastname'],
                                              ENT_QUOTES, 'UTF-8').'"'
        .' data-class="'.($classid ?
                          htmlspecialchars($classes[$classid]['class'], ENT_QUOTES, 'UTF-8') : '').'"'
        .' data-rank="'.($rankid ?
                          htmlspecialchars($ranks[$rankid]['rank'], ENT_QUOTES, 'UTF-8') : '').'"'
        .'>';
    echo '<span>'.htmlspecialchars($row['awardname'], ENT_QUOTES, 'UTF-8').'</span>';
    echo '<p><strong>'.$row['carnumber'].':</strong> ';
    echo htmlspecialchars($row['firstname'].' '.$row['lastname'], ENT_QUOTES, 'UTF-8');
    echo '</p>';
    echo '</li>';
}
?>
</ul>
</div>

<div class="presenter">

<div id="kiosk-summary">
<?php
    $nkiosks = read_single_value('SELECT COUNT(*) FROM Kiosks'
                                 .' WHERE page LIKE \'%award%present%\'', array());
    if ($nkiosks == 0) {
      echo '<h3>NOTE:</h3>';
      echo '<h3>There are NO kiosks ready for award presentation.</h3>';
      echo '<p class="moot">Selections on this dashboard won\'t have any observable effect.</p>';
      echo '<p class="moot">Visit the <a href="kiosk-dashboard.php">Kiosk Dashboard</a> to assign displays.</p>';
    }
?>
</div>

<h3 id="awardname"></h3>

<h3 id="classname"></h3>
<h3 id="rankname"></h3>

<h3 id="recipient"></h3>

<div class="presenter-inner hidden">
  <input type="checkbox" data-role="flipswitch"
        id="reveal-checkbox"
        data-on-text="Showing"
        data-off-text="Hidden"
        onchange="on_reveal();"/>
</div>

<div class="reset-footer block_buttons">
    <input type="button" data-enhanced="true" value="Clear" onclick="on_clear_awards()"/>
</div>

</div>

</div>
</body>
</html>
