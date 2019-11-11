<?php

require_once('inc/standings.inc');
require_once('inc/ordinals.inc');
require_once('inc/aggregate_round.inc');

function maybe_populate_award_types() {
  if (read_single_value('SELECT COUNT(*) FROM AwardTypes') == 0) {
    run_sql_script('award-type-values');
  }
}

define('AD_HOC_AWARDTYPEID', 6);

// list($classes, $classseq, $ranks, $rankseq) = classes_and_ranks();
//   $classes:  classid => {'class', 'constituents', 'ntrophies', 'seq'}
//   $classseq: ordered classids
//   $ranks:    rankid => {'class', 'classid', 'rank', 'rankid', 'seq'}
//   $rankseq:  ordered rankids
//
// Note that (of course) $ranks and $rankseq won't account for aggregate
// classes, which don't have any ranks.
function classes_and_ranks() {
  global $db;
  $use_subgroups = read_raceinfo_boolean('use-subgroups');
  $n_den_trophies = read_raceinfo('n-den-trophies', 3);
  $n_pack_trophies = read_raceinfo('n-pack-trophies', 3);

  $classes = array();  // classid => {'class', 'seq'}
  $classseq = array(); // ordered classids
  $ranks = array();    // rankid => {'class', 'rank'}
  $rankseq = array();  // ordered rankids
  foreach ($db->query('SELECT Classes.classid, class, rankid, rank,'
                      .(schema_version() >= 3
                        ? 'constituents, ntrophies'
                        : '\'\' AS constituents, -1 AS ntrophies')
                      .' FROM Classes'
                      .' LEFT JOIN Ranks ON Classes.classid = Ranks.classid'
                      .' ORDER BY '.(schema_version() >= 2
                                     ? 'Classes.sortorder, Ranks.sortorder, '
                                     : '')
                      .'class, rank') as $row) {
    if (!isset($classes[$row['classid']])) {
      $classseq[] = $row['classid'];
      $classes[$row['classid']] = array('class' => $row['class'],
                                        'constituents' => explode(',', $row['constituents']),
                                        'ntrophies' => $row['ntrophies'],
                                        'seq' => count($classseq));
    }
    if (isset($row['rankid'])) {
      $rankseq[] = $row['rankid'];
      $ranks[$row['rankid']] = array('class' => $row['class'],
                                     'classid' => $row['classid'],
                                     'rank' => $row['rank'],
                                     'rankid' => $row['rankid'],
                                     'seq' => count($rankseq));
    }
  }
  return array($classes, $classseq, $ranks, $rankseq);
}

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

// Within a rank (if $rankid), or a class (if $classid), or the supergroup (if
// neither $classid nor $rankid), add to the argument $awards list entries for
// the top $n speed trophies.  Returns the highest award sort value of the speed
// awards added.
function add_speed_group($n, $bias, $classid, $rankid, $label, &$standings, &$awards) {
  $finishers = top_finishers(@$classid, @$rankid, $standings);
  for ($p = 0; $p < $n; ++$p) {
    if (!isset($finishers[$p])) {
      continue;
    }
    for ($i = 0; $i < count($finishers[$p]); ++$i) {
      $racerid = $finishers[$p][$i];
      $sort = $bias + $p;
      $row = get_racer_details($racerid);
      $awards[] = array('classid' => @$classid,
                        'rankid' => @$rankid,
                        'awardkey' => 'speed-'.(1 + $p)
                            .(count($finishers[$p]) > 1 ? chr(ord('a') + $i) : '')
                            .(isset($classid) ? '-'.$classid : '')
                            .(isset($rankid) ? '-'.$rankid : ''),
                        'awardname' => nth_fastest(1 + $p, $label),
                        // TODO Hard-wired constants, ugh
                        'awardtype' => 'Speed Trophy',
                        'awardtypeid' => 5,
                        'sort' => $sort,
                        'racerid' => $racerid,
                        'firstname' => $row['firstname'],
                        'lastname' => $row['lastname'],
                        'carnumber' => $row['carnumber'],
                        'carname' => $row['carname']);
    }
  }
}

// Returns bias for 'sort' values, so the speed awards will be ordered first.
//
// TODO -- Maybe make one pass for top finishers in each rank/class/overall, and
// do away with add_speed_group entirely.
function add_speed_awards(&$awards) {
  $standings = final_standings();
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
  add_speed_group($n_pack_trophies, $bias_for_pack, null, null,
                  supergroup_label(), $standings, $awards);

  // $ordinal_class is a 1-based index of classes.
  $ordinal_class = 0;
  foreach (array_reverse(aggregate_classes()) as $agg_class) {
    ++$ordinal_class;
    $bias_for_den = $bias_for_pack + $ordinal_class * $reserved_per_den;
    $ntrophies = $agg_class['ntrophies'];
    if ($ntrophies < 0) $ntrophies = $n_den_trophies;
    add_speed_group($ntrophies, $bias_for_den, $agg_class['classid'], null,
                    $agg_class['class'], $standings, $awards);
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
      add_speed_group($n_den_trophies, $bias_for_den, $classid, null, $rank['class'], $standings,
                      $awards);
    }

    ++$rank_within_den;
    $bias_for_rank = $bias_for_den + $n_den_trophies + $rank_within_den * $reserved_per_rank;
    add_speed_group($n_rank_trophies, $bias_for_rank, null, $r, $rank['rank'], $standings, $awards);
  }

  return $reserved_overall;
}

?>