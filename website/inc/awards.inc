<?php
require_once('inc/classes.inc');
require_once('inc/partitions.inc');
require_once('inc/standings.inc');
require_once('inc/ordinals.inc');
require_once('inc/aggregate_round.inc');
require_once('inc/schema_version.inc');
require_once('inc/voterid.inc');

function maybe_populate_award_types() {
  if (read_single_value('SELECT COUNT(*) FROM AwardTypes') == 0) {
    run_sql_script('award-type-values');
  }
}

define('AD_HOC_AWARDTYPEID', 6);

// Returns all the awards in the Awards table, whether or not it has an assigned
// recipient.  Note that this does NOT include speed awards.  Each entry in the
// returned array has:
// {classid (of the award), rankid (of the award),
//  awardid, awardkey, awardname, awardtype, awardtypeid, sort (of the award),
//  racerid, firstname, lastname, carnumber, carname},
// in award.sort order.
function all_awards($include_ad_hoc, $sort_bias = 0) {
  global $db;
  $recipients = array();
  foreach ($db->query('SELECT Awards.racerid, firstname, lastname, carnumber, carname'
                      .' FROM '.inner_join('Awards', 'RegistrationInfo',
                                           'Awards.racerid = RegistrationInfo.racerid')) as $racer) {
    $recipients[$racer['racerid']] = array('racerid' => $racer['racerid'],
                                           'firstname' => $racer['firstname'],
                                           'lastname' => $racer['lastname'],
                                           'carnumber' => $racer['carnumber'],
                                           'carname' => $racer['carname']);
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
          'carname' => @$racer['carname'],
          'sort' => $sort_bias + $row['sort']);
  }
  return $awards;
}

// Returns {racerid, firstname, lastname, carnumber, carname} given racerid
function get_racer_details($racerid) {
  return read_single_row('SELECT racerid, carnumber, lastname, firstname, carname'
                         .' FROM RegistrationInfo'
                         .' WHERE racerid = :racerid',
                         array(':racerid' => $racerid),
                         PDO::FETCH_ASSOC);
}

// Compute $n speed trophy winners, within a rank (if $rankid), or a class (if
// $classid), or the supergroup (if neither $classid nor $rankid).  Awards are
// added to the argument $awards list entries.
//
// $bias gives an offset for 'sort' values so that awards for this group will order
// correctly relative to other groups.
//
// $no_trophies_for is an array of racerids.  For organizations that have a
// "only one trophy per racer" policy, these are racers who have already earned
// other trophies.  If a speed award would otherwise go to one of the racers in
// this list, it is instead bestowed on the next eligible racer.
// Racer IDs are also added to $no_trophies_for as they receive awards.
function add_speed_group($n, $bias, $classid, $rankid, $label, &$no_trophies_for,
                         $finishers, &$awards) {
  // $place is the trophy "place" (i.e., $place = 0 for 1st place).
  // $p is the position in the $finishers ladder, which gives the true place in the standings.
  $place = 0;
  for ($p = 0; $place < $n; ++$p) {
    if (!isset($finishers[$p])) {
      ++$place;
      continue;
    }

    $recipients = array_diff($finishers[$p], $no_trophies_for);
    if (count($recipients) == 0) {
      // This means the recipient(s) for this trophy are on the no_trophies_for,
      // and that we should progress to the next tier in the ladder, but without
      // advancing the $place variable.  E.g., the first place trophy should be
      // presented to the racer in second place for standings.
      continue;
    }

    for ($i = 0; $i < count($recipients); ++$i) {
      $racerid = $recipients[$i];
      $sort = $bias + $place;
      $row = get_racer_details($racerid);
      $awards[] = array('classid' => @$classid,
                        'rankid' => @$rankid,
                        'place' => 1 + $place,
                        'awardkey' => 'speed-'.(1 + $place)
                            .(count($finishers[$p]) > 1 ? chr(ord('a') + $i) : '')
                            .(isset($classid) ? '-'.$classid : '')
                            .(isset($rankid) ? '-'.$rankid : ''),
                        'awardname' => nth_fastest(1 + $place, $label),
                        // TODO Hard-wired constants, ugh
                        'awardtype' => 'Speed Trophy',
                        'awardtypeid' => 5,
                        'sort' => $sort,
                        'racerid' => $racerid,
                        'firstname' => $row['firstname'],
                        'lastname' => $row['lastname'],
                        'carnumber' => $row['carnumber'],
                        'carname' => $row['carname']);
      $no_trophies_for[] = $racerid;
    }
    ++$place;
  }
}

// Returns bias for 'sort' values, so the speed awards will be ordered first.
//
// TODO -- Maybe make one pass for top finishers in each rank/class/overall, and
// do away with add_speed_group entirely.
function add_speed_awards(&$awards) {
  $one_trophy_per = read_raceinfo_boolean('one-trophy-per');
  $pack_aggregate_id = read_raceinfo('full-field-calc', 0);

  $standings = new StandingsOracle();

  $no_trophies_for = array();

  $n_pack_trophies = read_raceinfo('n-pack-trophies', 3);
  $n_den_trophies = read_raceinfo('n-den-trophies', 3);
  $n_rank_trophies = read_raceinfo('n-rank-trophies', 0);

  list($classes, $classseq, $ranks, $rankseq) = classes_and_ranks();
  
  $max_den_trophies = $n_den_trophies;
  foreach ($classes as $classid => $cl) {
    $max_den_trophies = max($max_den_trophies, $cl['ntrophies']);
  }

  $max_ranks_per_den = read_single_value(
    'SELECT COUNT(*) FROM Ranks GROUP BY classid ORDER BY COUNT(*) desc');

  $reserved_per_rank = $n_rank_trophies;
  $reserved_per_den = $max_den_trophies + $reserved_per_rank * $max_ranks_per_den;
  $reserved_overall = $n_pack_trophies + $reserved_per_den * count($classes);

  $bias_for_pack = 0;
  if ($standings->has_full_field_standings()) {
    add_speed_group($n_pack_trophies, $bias_for_pack, null, null,
                    supergroup_label(), $no_trophies_for,
                    $standings->award_ladder_full_field(), $awards);
  } else if ($pack_aggregate_id > 0) {
    // Use the specified aggregate class results to determine pack-level winners.
    // In the loop below iterating the aggregate classes, we'll skip this one.
    add_speed_group($n_pack_trophies, $bias_for_pack, $pack_aggregate_id, null,
                    supergroup_label(), $no_trophies_for,
                    $standings->award_ladder_class($pack_aggregate_id), $awards);
  }

  $aggregate_classes = aggregate_classes();
  // $ordinal_class is a 1-based index of classes.
  $ordinal_class = 0;
  foreach (topological_sort($aggregate_classes) as $agg_class) {
    ++$ordinal_class;
    $bias_for_den = $bias_for_pack + $ordinal_class * $reserved_per_den;
    if ($agg_class['classid'] != $pack_aggregate_id) {
      $group_name = $agg_class['class'];
      $ntrophies = $agg_class['ntrophies'];
      if ($ntrophies < 0) $ntrophies = $n_den_trophies;
      if (!$one_trophy_per) $no_trophies_for = array();
      add_speed_group($ntrophies, $bias_for_den, $agg_class['classid'], null,
                      $group_name, $no_trophies_for,
                      $standings->award_ladder_class($agg_class['classid']), $awards);
    }
  }

  $classid = -1;
  // $rank_within_den is a 1-based index of ranks WITHIN THE CURRENT CLASS.
  $rank_within_den = 0;
  foreach ($rankseq as $r) {
    $rank = $ranks[$r];

    if ($rank['classid'] != $classid) {
      $classid = $rank['classid'];
      ++$ordinal_class;
      $bias_for_den = $bias_for_pack + $ordinal_class * $reserved_per_den;
      $rank_within_den = 0;
      if (!$one_trophy_per) $no_trophies_for = array();
      add_speed_group($n_den_trophies, $bias_for_den, $classid, null, $rank['class'],
                      $no_trophies_for,
                      $standings->award_ladder_class($classid), $awards);
    }

    ++$rank_within_den;
    $bias_for_rank = $bias_for_den + $n_den_trophies + $rank_within_den * $reserved_per_rank;
    if (!$one_trophy_per) $no_trophies_for = array();
    add_speed_group($n_rank_trophies, $bias_for_rank, $classid, $r, $rank['rank'],
                    $no_trophies_for,
                    $standings->award_ladder_subgroup($r), $awards);
  }

  return $reserved_overall;
}

// For each award, add eligible-classids and eligible-rankids fields.
function mark_award_eligibility(&$awards) {
  list($classes, $classseq, $ranks, $rankseq) = classes_and_ranks();
  $agg_classes = aggregate_classes();
  compute_aggregate_constituents_tc($agg_classes);
  foreach ($awards as &$award) {
    // For awards on aggregate classes, the award can be given to any members of
    // the native classes that comprise the aggregate class, perhaps indirectly.
    if ($award['classid'] == 0) {
      $award['eligible-classids'] = $classseq;  // All classids
    } else if (isset($agg_classes[$award['classid']])) {
      $agg = &$agg_classes[$award['classid']];
      $award['eligible-classids'] = $agg['constituents_tc'];
    } else {
      $award['eligible-classids'] = array($award['classid']);
    }

    if ($award['rankid'] != 0) {
      $award['eligible-rankids'] = array($award['rankid']);
    } else if ($award['classid'] != 0 && isset($agg_classes[$award['classid']])) {
      $agg = &$agg_classes[$award['classid']];
      if (count($agg['rankids']) > 0) {
        $award['eligible-rankids'] = $agg['rankids'];
      } else {
        $award['eligible-rankids'] = $rankseq;  // All rankids
      }
    } else {
      $award['eligible-rankids'] = $rankseq;  // All rankids
    }
  }
}

function mark_votes_and_balloting(&$awards) {
  global $db;
  if (schema_version() < BALLOTING_SCHEMA) {
    foreach ($awards as &$award) {
      $award['ballot_depth'] = 0;
      $award['votes'] = array();
    }
    return;
  }

  $depth_stmt = $db->prepare('SELECT depth FROM BallotAwards'
                             .'     WHERE awardid = :awardid'
                             .'      AND ballotid = :ballotid');
  $votes_stmt = $db->prepare('SELECT Votes.racerid, COUNT(*) AS score,'
                             .' firstname, lastname, carnumber'
                             .' FROM '.inner_join('Votes', 'RegistrationInfo',
                                                  'Votes.racerid = RegistrationInfo.racerid')
                             .' WHERE Votes.awardid = :awardid'
                             .' GROUP BY Votes.racerid'
                             .' ORDER BY COUNT(*) DESC');

  foreach ($awards as &$award) {
    $depth_stmt->execute(array(':awardid' => $award['awardid'],
                               ':ballotid' => BALLOTID));
    $depth = $depth_stmt->fetch(PDO::FETCH_ASSOC);
    $award['ballot_depth'] = ($depth === false) ? 0 : $depth['depth'];
    $depth_stmt->closeCursor();

    $votes_stmt->execute(array(':awardid' => $award['awardid']));
    $award['votes'] = $votes_stmt->fetchAll(PDO::FETCH_ASSOC);
  }
}

?>
