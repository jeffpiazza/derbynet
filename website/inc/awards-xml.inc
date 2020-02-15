<?php

require_once('inc/awards.inc');
require_once('inc/classes.inc');
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
  $agg_classes = aggregate_classes();
  compute_aggregate_constituents_tc($agg_classes);
  foreach (all_awards($include_ad_hoc) as $award) {
    echo '<award';
    echo ' awardid="'.$award['awardid'].'"';
    echo ' awardname="'.htmlspecialchars($award['awardname'], ENT_QUOTES, 'UTF-8').'"';
    echo ' awardtypeid="'.$award['awardtypeid'].'"';
    echo ' adhoc="'.($award['awardtypeid'] == AD_HOC_AWARDTYPEID ? 1 : 0).'"';
    echo ' classid="'.@$award['classid'].'"';
    // For awards on aggregate classes, the award can be given to any members of
    // the native classes that comprise the aggregate class, perhaps indirectly.
    // For these cases, the 'eligible' attribute names all those native classes.
    // (Otherwise, a racer's eligibility for an award is determined by the
    // classid attribute, which must be either 0 or the same as the racer's
    // native class.)
    if (isset($award['classid']) && isset($agg_classes[$award['classid']])) {
      echo ' eligible="'.implode(',', $agg_classes[$award['classid']]['constituents_tc']) .'"';
    }
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
function emit_one_speed_award($racerid, $category, $place) {
  echo "<speed-award racerid=\"".$racerid."\" category=\"".$category."\" place=\"".$place."\"/>\n";
}

function emit_speed_award_group($n, $category, &$finishers) {
  for ($p = 0; $p < $n; ++$p) {
    foreach ($finishers[$p] as $racerid) {
      emit_one_speed_award($racerid, $category, 1 + $p);
    }
  }
}

function emit_speed_awards() {
  $n_per_den = read_raceinfo('n-den-trophies', 3);
  $n_per_rank = read_raceinfo('n-rank-trophies', 0);

  list($finishers, $parallel) = compute_all_standings(result_summary(), /* finalized_only */ true);

  emit_speed_award_group(read_raceinfo('n-pack-trophies', 3), 'supergroup',
                         $finishers['supergroup']);
  
  list($classes, $classseq, $ranks, $rankseq) = classes_and_ranks();
  foreach ($classseq as $c) {
    $ntrophies = $classes[$c]['ntrophies'];
    if ($ntrophies < 0) $ntrophies = $n_per_den;
    emit_speed_award_group($ntrophies, 'group', $finishers[standings_key_class($c)]);
  }
  foreach ($rankseq as $r) {
    emit_speed_award_group($n_per_rank, 'subgroup', $finishers[standings_key_subgroup($r)]);
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