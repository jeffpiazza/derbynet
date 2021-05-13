<?php

require_once('inc/awards.inc');
require_once('inc/classes.inc');
require_once('inc/json-classes.inc');
require_once('inc/standings.inc');

function json_award_types($include_ad_hoc) {
  global $db;
  $stmt = $db->query("SELECT awardtypeid, awardtype"
                     ." FROM AwardTypes"
                     .($include_ad_hoc ? "" : (" WHERE awardtypeid <> ".AD_HOC_AWARDTYPEID))
                     ." ORDER BY awardtypeid");
  $awardtypes = array();
  if ($stmt) {
    foreach ($stmt as $atype) {
      $awardtypes[] = array('awardtypeid' => $atype['awardtypeid'] + 0,
                            'awardtype' => $atype['awardtype']);
    }
  }
  return $awardtypes;
}

function json_all_awards($include_ad_hoc) {
  global $db;
  $all_awards = all_awards($include_ad_hoc);
  mark_award_eligibility($all_awards);
  mark_votes_and_balloting($all_awards);
  $jawards = array();
  foreach ($all_awards as $award) {
    $votes = array();
    foreach ($award['votes'] as $vote) {
      $votes[] = array('racerid' => $vote['racerid'] + 0,
                       'score' => $vote['score'] + 0,
                       'carnumber' => $vote['carnumber'],
                       'firstname' => $vote['firstname'],
                       'lastname' => $vote['lastname']);
    }
    $jawards[] = array('awardid' => $award['awardid'] + 0,
                       'awardname' => $award['awardname'],
                       'awardtype' => $award['awardtype'],
                       'awardtypeid' => $award['awardtypeid'] + 0,
                       'adhoc' => ($award['awardtypeid'] == AD_HOC_AWARDTYPEID),
                       'classid' => @$award['classid'] + 0,
                       'eligible-classids' => $award['eligible-classids'],
                       'eligible-rankids' => $award['eligible-rankids'],
                       'rankid' => @$award['rankid'] + 0,
                       'racerid' => $award['racerid'] + 0,
                       'firstname' => $award['firstname'] ? $award['firstname'] : '',
                       'lastname' => $award['lastname'] ? $award['lastname'] : '',
                       'carnumber' => $award['carnumber'],
                       'sort' => $award['sort'] + 0,
                       'ballot_depth' => $award['ballot_depth'] + 0,
                       'votes' => $votes);
  }
  return $jawards;
}

function json_speed_awards() {
  $awards = array();
  add_speed_awards($awards);
  $jawards = array();
  foreach ($awards as $award) {
    $a = array('type' => 'speed-award',  // TODO
               'racerid' => $award['racerid'] + 0,
               'key' => $award['awardkey'],
               'place' => $award['place'] + 0);
    if (isset($award['rankid'])) {
      $a['category'] = 'subgroup';
      $a['classid'] = $award['classid'] + 0;
      $a['rankid'] = $award['rankid'] + 0;
    } else if (isset($award['classid'])) {
      $a['category'] = 'group';
      $a['classid'] = $award['classid'] + 0;
    } else {
      $a['category'] = 'supergroup';
    }
    $jawards[] = $a;
  }
  return $jawards;
}

function json_out_awards($include_adhoc) {
  json_out('award-types', json_award_types($include_adhoc));
  json_out('awards', json_all_awards($include_adhoc));
  json_out('speed-awards', json_speed_awards());
  json_out('classes', json_all_classes());
}
?>