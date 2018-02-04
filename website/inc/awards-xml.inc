<?php

require_once('inc/awards.inc');
require_once('inc/classes-xml.inc');

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

function emit_award_list($include_ad_hoc) {
  echo "<awards>\n";
  emit_all_classes();
  emit_award_types($include_ad_hoc);
  emit_all_awards($include_ad_hoc);
  echo "</awards>\n";
}
?>