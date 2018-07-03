<?php

require_once('inc/awards.inc');
require_once('inc/classes-xml.inc');
require_once('inc/standings.inc');

function emit_award_types($include_ad_hoc) {
  global $db;
  foreach ($db->query("SELECT awardtypeid, awardtype"
                      ." FROM AwardTypes"
                      .($include_ad_hoc ? "" : (" WHERE awardtypeid <> ".AD_HOC_AWARDTYPEID))
                      ." ORDER BY awardtypeid") as $atype) {
    echo '<awardtype';
    echo ' awardtypeid="'.$atype['awardtypeid'].'"';
    echo ' awardtype="'.htmlspecialchars($atype['awardtype'], ENT_QUOTES, 'UTF-8').'"';
    echo "/>\n";
  }
}

function emit_all_awards($include_ad_hoc) {
  global $db;
  foreach (all_awards($include_ad_hoc) as $award) {
    echo '<award';
    echo ' awardid="'.$award['awardid'].'"';
    echo ' awardname="'.htmlspecialchars($award['awardname'], ENT_QUOTES, 'UTF-8').'"';
    echo ' awardtypeid="'.$award['awardtypeid'].'"';
    echo ' adhoc="'.($award['awardtypeid'] == AD_HOC_AWARDTYPEID ? 1 : 0).'"';
    echo ' classid="'.@$award['classid'].'"';
    echo ' rankid="'.@$award['rankid'].'"';
    echo ' racerid="'.$award['racerid'].'"';
    echo ' firstname="'.htmlspecialchars($award['firstname'], ENT_QUOTES, 'UTF-8').'"';
    echo ' lastname="'.htmlspecialchars($award['lastname'], ENT_QUOTES, 'UTF-8').'"';
    echo ' carnumber="'.$award['carnumber'].'"';
    echo ' sort="'.$award['sort'].'"';
    echo "/>\n";
  }
}

// $category is one of 'supergroup', 'group', 'subgroup'
function emit_one_speed_award(&$row, $category, $place) {
  echo "<speed-award racerid=\"".$row['racerid']."\" category=\"".$category."\" place=\"".$place."\"/>\n";
}

function emit_speed_awards() {
  $n_per_pack = read_raceinfo('n-pack-trophies', 3);
  $n_per_den = read_raceinfo('n-den-trophies', 3);
  $n_per_rank = read_raceinfo('n-rank-trophies', 0);

  $top_pack = array();  // List of racerids
  $top_den = array();   // List of classid => list of racerids
  $top_rank = array();  // List of rankid => list of racerids
  foreach (final_standings(/* completed only */ true) as $row) {
    if ($row['for_supergroup']) {
      if (count($top_pack) < $n_per_pack) {
        $top_pack[] = $row['racerid'];
        emit_one_speed_award($row, 'supergroup', count($top_pack));
      }
    }
    if ($row['for_group']) {
      $racerid = $row['racerid'];
      $classid = $row['classid'];
      $rankid = $row['rankid'];
      if (!isset($top_den[$classid])) {
        $top_den[$classid] = array();
      }
      if (count($top_den[$classid]) < $n_per_den) {
        $top_den[$classid][] = $racerid;
        emit_one_speed_award($row, 'group', count($top_den[$classid]));
      }
      if (!isset($top_rank[$rankid])) {
        $top_rank[$rankid] = array();
      }
      if (count($top_rank[$rankid]) < $n_per_rank) {
        $top_rank[$rankid][] = $racerid;
        emit_one_speed_award($row, 'subgroup', count($top_rank[$rankid]));
      }
    }
  }
}

function emit_award_list($include_ad_hoc) {
  echo "<awards>\n";
  emit_all_classes();
  emit_award_types($include_ad_hoc);
  emit_all_awards($include_ad_hoc);
  emit_speed_awards();
  echo "</awards>\n";
}
?>