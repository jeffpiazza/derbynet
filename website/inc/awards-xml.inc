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
  $all_awards = all_awards($include_ad_hoc);
  mark_award_eligibility($all_awards);
  foreach ($all_awards as $award) {
    echo '<award';
    echo ' awardid="'.$award['awardid'].'"';
    echo ' awardname="'.htmlspecialchars($award['awardname'], ENT_QUOTES, 'UTF-8').'"';
    echo ' awardtypeid="'.$award['awardtypeid'].'"';
    echo ' adhoc="'.($award['awardtypeid'] == AD_HOC_AWARDTYPEID ? 1 : 0).'"';
    echo ' classid="'.@$award['classid'].'"';
    echo ' eligible-classids="'.implode(',', $award['eligible-classids']).'"';
    echo ' eligible-rankids="'.implode(',', $award['eligible-rankids']).'"';
    echo ' rankid="'.@$award['rankid'].'"';
    echo ' racerid="'.$award['racerid'].'"';
    echo ' firstname="'.htmlspecialchars($award['firstname'], ENT_QUOTES, 'UTF-8').'"';
    echo ' lastname="'.htmlspecialchars($award['lastname'], ENT_QUOTES, 'UTF-8').'"';
    echo ' carnumber="'.$award['carnumber'].'"';
    echo ' sort="'.$award['sort'].'"';
    echo "/>\n";
  }
}

function emit_speed_awards() {
  $awards = array();
  add_speed_awards($awards);
  foreach ($awards as $award) {
    echo "<speed-award racerid=\"$award[racerid]\" key=\"$award[awardkey]\"";
    if (isset($award['rankid'])) {
      echo " category=\"subgroup\" classid=\"$award[classid]\" rankid=\"$award[rankid]\"";
    } else if (isset($award['classid'])) {
      echo " category=\"group\" classid=\"$award[classid]\"";
    } else {
      echo " category=\"supergroup\"";
    }
    echo " place=\"$award[place]\"/>\n";
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