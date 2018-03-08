<?php

require_once('inc/standings.inc');
require_once('inc/ordinals.inc');

function maybe_populate_award_types() {
  if (read_single_value('SELECT COUNT(*) FROM AwardTypes') == 0) {
    run_sql_script('award-type-values');
  }
}

define('AD_HOC_AWARDTYPEID', 6);

// list($classes, $classseq, $ranks, $rankseq) = classes_and_ranks();
function classes_and_ranks() {
  global $db;
  $use_subgroups = read_raceinfo_boolean('use-subgroups');
  $n_den_trophies = read_raceinfo('n-den-trophies', 3);
  $n_pack_trophies = read_raceinfo('n-pack-trophies', 3);

  $classes = array();  // classid => {'class'}
  $classseq = array(); // ordered classids
  $ranks = array();    // rankid => {'class', 'rank'}
  $rankseq = array();  // ordered rankids
  foreach ($db->query('SELECT Classes.classid, class, rankid, rank'
                      .' FROM Classes'
                      .' LEFT JOIN Ranks ON Classes.classid = Ranks.classid'
                      .' ORDER BY '.(schema_version() >= 2
                                     ? 'Classes.sortorder, Ranks.sortorder, '
                                     : '')
                      .'class, rank') as $row) {
    if (!isset($classes[$row['classid']])) {
      $classseq[] = $row['classid'];
      $classes[$row['classid']] = array('class' => $row['class']);
    }
    $rankseq[] = $row['rankid'];
    $ranks[$row['rankid']] = array('class' => $row['class'],
                                   'classid' => $row['classid'],
                                   'rank' => $row['rank'],
                                   'rankid' => $row['rankid']);
  }
  return array($classes, $classseq, $ranks, $rankseq);
}

// Returns all the awards in the Awards table.  Note that this does NOT include
// speed awards.
function all_awards($include_ad_hoc) {
  global $db;
  $recipients = array();
  foreach ($db->query('SELECT Awards.racerid, firstname, lastname, carnumber'
                      .' FROM '.inner_join('Awards', 'RegistrationInfo',
                                           'Awards.racerid = RegistrationInfo.racerid')) as $racer) {
    $recipients[$racer['racerid']] = array('racerid' => $racer['racerid'],
                                           'firstname' => $racer['firstname'],
                                           'lastname' => $racer['lastname'],
                                           'carnumber' => $racer['carnumber']);
  }

  $awards = array();
  foreach ($db->query('SELECT awardid, awardname, awardtype,'
                      .' Awards.awardtypeid, Awards.classid, Awards.rankid, racerid, sort'
                      .' FROM '.inner_join('Awards', 'AwardTypes',
                                           'Awards.awardtypeid = AwardTypes.awardtypeid')
                      .($include_ad_hoc ? '' : (' WHERE Awards.awardtypeid <> '.AD_HOC_AWARDTYPEID))
                      .' ORDER BY sort, awardtype, awardname') as $row) {
    $racer = @$recipients[$row['racerid']];
    $awards[] =
    array('classid' => @$row['classid'],
          'rankid' => @$row['rankid'],
          'awardid' => $row['awardid'],
          'awardkey' => 'award-'.$row['awardid'],
          'awardname' => $row['awardname'],
          'awardtype' => $row['awardtype'],
          'awardtypeid' => $row['awardtypeid'],
          'racerid' => $row['racerid'],
          'firstname' => @$racer['firstname'],
          'lastname' => @$racer['lastname'],
          'carnumber' => @$racer['carnumber'],
          'sort' => $row['sort']);
  }
  return $awards;
}
?>