<?php
require_once('inc/classes.inc');
require_once('inc/standings.inc');
require_once('inc/ordinals.inc');
require_once('inc/aggregate_round.inc');

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
                         &$finishers, &$awards) {
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
  $result_summary = result_summary();
  list($finishers, $parallel) = compute_all_standings(result_summary());

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
  add_speed_group($n_pack_trophies, $bias_for_pack, null, null,
                  supergroup_label(), $no_trophies_for,
                  $finishers['supergroup'], $awards);

  // $ordinal_class is a 1-based index of classes.
  $ordinal_class = 0;
  foreach (array_reverse(aggregate_classes()) as $agg_class) {
    ++$ordinal_class;
    $bias_for_den = $bias_for_pack + $ordinal_class * $reserved_per_den;
    $ntrophies = $agg_class['ntrophies'];
    if ($ntrophies < 0) $ntrophies = $n_den_trophies;
    if (!$one_trophy_per) $no_trophies_for = array();
    add_speed_group($ntrophies, $bias_for_den, $agg_class['classid'], null,
                    $agg_class['class'], $no_trophies_for,
                    $finishers[standings_key_class($agg_class['classid'])], $awards);
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
                      $finishers[standings_key_class($classid)], $awards);
    }

    ++$rank_within_den;
    $bias_for_rank = $bias_for_den + $n_den_trophies + $rank_within_den * $reserved_per_rank;
    if (!$one_trophy_per) $no_trophies_for = array();
    add_speed_group($n_rank_trophies, $bias_for_rank, $classid, $r, $rank['rank'],
                    $no_trophies_for,
                    $finishers[standings_key_subgroup($r)], $awards);
  }

  return $reserved_overall;
}

?>