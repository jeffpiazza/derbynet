<?php

require_once('inc/form_groups_by_rule.inc');
require_once('inc/partitions.inc');
require_once('inc/class_add_or_delete.inc');
require_once('inc/rank_add_or_delete.inc');

// file_get_contents($_FILES['upload_file']['tmp_name']) to get an uploaded file content, or
// accept preferences as a pasted-in text field.

// Settings are just (tag) = (val), where tag is the string used in
// write_settings, often the same as the itemkey written to the RaceInfo table.
//
// AWARD = (award name)
// SEGMENT = (partition name)
//
// GROUP = (group name) starts a context that extends to the next ENDGROUP.
// Within a group, SUBGROUPs, SEGMENTs, AWARDs, or NTROPHIES may appear within
// a group.
//
///// Not currently used
// SUBGROUP = (subgroup name) starts a context that extends to the next
// ENDSUBGROUP.  Subgroups must appear only within a group.  SEGMENTs, AWARDs,
// or NTROPHIES may appear within a subgroup.
//
// AGGREGATE = (aggregate group name) starts a context that extends to the next
// ENDAGGREGATE.  Within an aggregate context, CONSTITUENTGROUP or
// CONSTITUENTSUBGROUPs may appear, but not a mix of the two.

function create_award($awardname, $awardtype, $classid, $rankid) {
  global $db;
  $awardname = trim($awardname);
  $stmt = $db->prepare('INSERT INTO Awards(awardname, awardtypeid, classid, rankid, racerid, sort)'
                       .' VALUES(:awardname, (SELECT awardtypeid FROM AwardTypes WHERE awardtype = :awardtype),'
                       .'        :classid, :rankid, :racerid,'
                       .'        (SELECT 1 + MAX(sort) FROM Awards))');
  $stmt->execute(array(':awardname' => $awardname,
                       ':awardtype' => $awardtype,
                       ':classid' => $classid,
                       ':rankid' => $rankid));
  record_action(array('action' => 'award.*create',
                      'awardname' => $awardname,
                      'awardtype' => $awardtype,
                      'classid' => $classid,
                      'rankid' => $rankid));
}

define('PREF_BOOL_SETTINGS', array(
  'use-subgroups' => 'use-subgroups',
  'one-trophy-per' => 'one-trophy-per',
  'reverse-lanes' => 'reverse-lanes',
  'show-car-photos-on-deck' => 'show-cars-on-deck',
  'show-car-photos-rr' => 'show-car-photos-rr',
  'show-racer-photos-rr' => 'show-racer-photos-rr',
  'upload-videos' => 'upload-videos',
  'use-master-sched' => 'use-master-sched',
  'use-points' => 'use-points',
  'use-xbs' => 'use-xbs',
  'warn-no-timer' => 'warn-no-timer',
));
define('PREF_STRING_SETTINGS', array(
  'n-lanes' => 'lane_count',
  'track-length' => 'track-length',
  'n-pack-trophies' => 'n-pack-trophies',
  'n-den-trophies' => 'n-den-trophies',
  'n-rank-trophies' => 'n-rank-trophies',
  'name-style' => 'name-style',
  'xbs-award' => 'xbs-award',
  'partition-label' => 'partition-label',
  'photos-on-now-racing' => 'photos-on-now-racing',
  'supergroup-label' => 'supergroup-label',
  'update-period' => 'update-period',
  // 'photo-dir' => 'photo-directory',
  // 'car-photo-dir' => 'car-photo-directory',
  // 'video-dir' => 'video-directory',
  // 'log-dir' => 'logs-directory',
  'replay-skipback' => 'replay-skipback',
  'replay-num-showings' => 'replay-num-showings',
  'replay-rate' => 'replay-rate',
  // 'timer-send-logs' => 'timer-send-logs',
  'unused-lane-mask' => 'unused-lane-mask',
  'max-runs-per-car' => 'max-runs-per-car',
  'rotation-schedule' => 'rotation-schedule',
  'finish-formatting' => 'finish-formatting',
  'scoring' => 'scoring',
  'now-racing-linger-ms' => 'now-racing-linger-ms',
  'racing_scene' => 'racing_scene',
  // 'images-dir' => 'images-dir',
  'ballot_password' => 'ballot_password',
  'group-formation-rule' => 'group-formation-rule',
  'car-numbering' => 'car-numbering',
));

// Calls json_success or json_failure as appropriate
function set_preferences($content) {
  $lineno = 0;
  $stack = array();  // ['GROUP', groupname], ['SUBGROUP', subgroupname], or ['AGGREGATE', aggregatename]
  $curr_group = null;
  $curr_subgroup = null;
  $curr_aggregate = null;
  $agg_classids = array();
  $agg_rankids = array();
  foreach (preg_split("/\r\n|\n|\r/", $content) as $line) {
    ++$lineno;
    $line = trim($line);
    $matched = false;
    if (strlen($line) == 0) continue;
    if ($line[0] == '#' || substr($line, 0, 2) == '//') continue;

    // unset($val);
    @list($line_tag, $val) = explode('=', $line, 2);
    $line_tag = trim($line_tag);
    unset($arg);
    $sp = strpos($line_tag, ' ');
    if ($sp !== false) {
      $arg = trim(substr($line_tag, $sp + 1));
      $line_tag = substr($line_tag, 0, $sp);
    }
    if (isset($val)) $val = trim($val);

    if ($line_tag == 'GROUP' && isset($val)) {
      if (!is_null($curr_group) || !is_null($curr_aggregate)) {
        json_failure('group-within-group', "At line $lineno, GROUP $val within another group or aggregate");
        return;
      }
      write_group_formation_rule('custom');
      $curr_group = find_or_create_class($val);
      array_push($stack, array($line_tag, $val));
      continue;
    }
    if ($line_tag == 'ENDGROUP') {
      if (is_null($curr_group)) {
        json_failure('endgroup-no-group', "At line $lineno, ENDGROUP without GROUP");
        return;
      }
      $curr_group = null;
      while ($stack[0][0] == 'SUBGROUP' || $stack[0][0] == 'GROUP') {
        array_pop($stack);
      }
      continue;
    }
    // SUBGROUP and ENDSUBGROUP aren't used (indistinguishable from partition)
    // if ($line_tag == 'SUBGROUP' && isset($val)) {
    //   if (is_null($curr_group)) {
    //     json_failure('subgroup-needs-a-group',
    //                  "At line $lineno, SUBGROUP $val without an enclosing GROUP");
    //     return;
    //   }
    //   if (!is_null($curr_subgroup)) {
    //     json_failure('subgroup-within-subgroup',
    //                  "At line $lineno, SUBGROUP $val within another subgroup");
    //     return;
    //   }
    //   // There's a risk here of creating a rank without a partition, which
    //   // we'd consider malformed.
    //   $curr_subgroup = find_or_create_rank($val, $curr_group, /*with_partition=*/false);
    //   array_push($stack, array($line_tag, $val));
    //   continue;
    // }
    // if ($line_tag == 'ENDSUBGROUP') {
    //   if (is_null($curr_subgroup)) {
    //     json_failure('endsubgroup-no-subgroup', "At line $lineno, ENDSUBGROUP without SUBGROUP");
    //     return;
    //   }
    //   $curr_subgroup = null;
    //   if ($stack[0][0] == 'SUBGROUP') {
    //     array_pop($stack);
    //   }
    //   continue;
    // }
    if ($line_tag == 'SEGMENT' && isset($val)) {
      $partitionid = read_single_value('SELECT partitionid FROM Partitions WHERE name = :val',
                                       array(':val' => $val));
      if ($partitionid !== false) {
        json_failure('division-exists', "At line $lineno , SEGMENT $val already exists.");
        return;
      }
      if (!is_null($curr_subgroup)) {
        create_partition_in_existing_rank($val, $curr_subgroup);
      } else if (!is_null($curr_group)) {
        $rankid = find_or_create_rank($val, $curr_group, /*with_partition=*/false);
        create_partition_in_existing_rank($val, $rankid);
      } else {
        // This will create a new class and rank corresponding to the new partition.
        // (We already checked that there isn't an existing partition with this name.)
        find_or_create_partition($val);
      }
      if (isset($arg)) { // sortorder value
        read_single_value('UPDATE Partitions SET sortorder = :arg WHERE name = :val',
                          array(':val' => $val,
                                ':arg' => $arg));
      }
      continue;
    }
    if ($line_tag == 'AWARD' && isset($val)) {
      create_award(trim($val), 'Design General',
                   (is_null($curr_group) || !is_null($curr_subgroup)) ? 0 : $curr_group,
                   is_null($curr_subgroup) ? 0 : $curr_subgroup);
      continue;
    }
    if ($line_tag == 'AGGREGATE' && isset($val)) {
      if (!is_null($curr_group)) {
        json_failure('aggregate-within', "At line $lineno, AGGREGATE $val within a group.");
        return;
      }
      $curr_aggregate = find_or_create_class($val, /*with_first_round=*/false);
      $agg_classids = array();
      $agg_rankids = array();
      array_push($stack, array($line_tag, $val));
      continue;
    }
    if ($line_tag == 'ENDAGGREGATE') {
      if (is_null($curr_aggregate)) {
        json_failure('endaggregate-no-aggregate', "At line $lineno, ENDAGGREGATE without an aggregate.");
        return;
      }
      if (count($agg_rankids) > 0) {
        read_single_value('UPDATE Classes SET rankids = :rankids, durable = 1 WHERE classid = :classid',
                          array(':classid' => $curr_aggregate,
                                ':rankids' => implode(',', $agg_rankids)));
      }
      if (count($agg_classids) > 0) {
        $agg_classids = array_unique($agg_classids);
        read_single_value('UPDATE Classes SET constituents = :classids, durable = 1 WHERE classid = :classid',
                          array(':classid' => $curr_aggregate,
                                ':classids' => implode(',', $agg_classids)));
        read_single_value('DELETE FROM ConstituentClasses WHERE dependent = :dep',
                          array(':dep' => $curr_aggregate));
        foreach ($agg_classids as $ante) {
          read_single_value('INSERT INTO ConstituentClasses(antecedent, dependent) VALUES(:a, :d)',
                            array(':a' => $ante,
                                  ':d' => $curr_aggregate));
        }
      }
      $curr_aggregate = null;
      if ($stack[0][0] == 'AGGREGATE') {
        array_pop($stack);
      }
      continue;
    }
    if ($line_tag == 'NTROPHIES' && isset($val)) {
      if (!is_null($curr_subgroup)) {
        // TODO Per-subgroup trophy counts
        json_failure('ntrohpies-in-subgroup', "At line $lineno, NTROPHIES for subgroup.");
        return;
      } else if (!is_null($curr_group) || !is_null($curr_aggregate)) {
        read_single_value('UPDATE Classes SET ntrophies = :ntrophies WHERE classid = :classid',
                          array(':ntrophies' => $val,
                                ':classid' => is_null($curr_group) ? $curr_aggregate : $curr_group));
        continue;
      } else {
        json_failure('ntrophies', "At line $lineno, NTROPHIES without context.");
        return;
      }
    }
    if ($line_tag ==  'CONSTITUENTGROUP' && isset($val)) {
      if (is_null($curr_aggregate)) {
        json_failure('constituentgroup', "At line $lineno, no aggregate");
        return;
      }
      if (count($agg_rankids) > 0) {
        json_failure('constituentgroup-rank', "At line $lineno, constituentgroup for ranks");
        return;
      }
      $agg_classids[] = find_or_create_class($val);
      continue;
    }
    if ($line_tag ==  'CONSTITUENTSUBGROUP' && isset($val)) {
      if (is_null($curr_aggregate)) {
        json_failure('constituentsubgroup', "At line $lineno, no aggregate");
        return;
      }
      if (count($agg_rankids) == 0 && count($agg_classids) > 0) {
        json_failure('constituentsubgroup-rank', "At line $lineno, constituentgroup for classes");
        return;
      }

      $rank = read_single_row('SELECT rankid, classid FROM Ranks WHERE rank = :name',
                              array(':name' => $val), PDO::FETCH_ASSOC);
      if ($rank === false) {
        json_failure('no-such-rank', "At line $lineno, constituentsubgroup $val not found.");
        return;
      }
      $agg_rankids[] = $rank['rankid'];
      $agg_classids[] = $rank['classid'];
      continue;
    }

    if (!isset($val)) {
      $key = @PREF_BOOL_SETTINGS[$line_tag];
      if (isset($key)) {
        write_raceinfo($key, '1');
        $matched = true;
        continue;
      } else if (substr($line_tag, 0, 3) == 'no-') {
        $key = @PREF_BOOL_SETTINGS[substr($line_tag, 3)];
        if (isset($key)) {
          write_raceinfo($key, '0');
          $matched = true;
          continue;
        }
      }
    } else {
      $key = @PREF_STRING_SETTINGS[$line_tag];
      if (isset($key)) {
        write_raceinfo($key, trim($val));
        $matched = true;
        continue;
      }
    }
    json_failure('prefs-syntax', "Preferences file syntax, line $lineno: $line ($val)");
    return;
  }

  // This confirms that Partitions have sortorder values of [1..n] (important for
  // car number assignments based on partition sortorder).
  if (read_single_value('SELECT MIN(sortorder) = 1 AND COUNT(*) = 1'
                        .' FROM Partitions p'
                        .' WHERE 1 <> (SELECT COUNT(*)'
                        .'             FROM Partitions q'
                        .'             WHERE q.sortorder = p.sortorder - 1)') <> 1) {
    json_failure('partition-suborder', 'Partition sortorder fails.');
    return;
  }

  json_success();
}

// Returns a string suitable for writing to a preferences file.
function dump_preferences() {
  global $db;
  $result = "# Preferences written ".date("Y-m-d")."\n\n";
  $group_rule = group_formation_rule();
  if ($group_rule != 'by-partition') {
    $result .= "group-formation-rule = $group_rule\n";
  }

  // TODO Custom award counts for classes, even for non-custom groups
  if ($group_rule == 'custom') {
    $result .= "\n## Groups\n";
    $prev = '';
    foreach ($db->query('SELECT class, rank, Classes.ntrophies,'
                        .'      Partitions.sortorder AS sortorder'
                        .' FROM Ranks JOIN Classes USING (classid)'
                        .'            JOIN Partitions USING (rankid)'
                        .' ORDER BY Classes.sortorder, Ranks.sortorder') as $row) {
      if ($row['class'] <> $prev) {
        if ($prev) {
          $result .= "ENDGROUP\n";
        }
        $result .= "GROUP = $row[class]\n";
        $prev = $row['class'];
        if ($row['ntrophies'] >= 0) {
          $result .= "  NTROPHIES = $row[ntrophies]\n";
        }
      }
      // $result .= "  SUBGROUP = $row[rank]\n";
      // $result .= "  ENDSUBGROUP\n";
      // Relying on 1-1 correspondence between ranks and partitions
      $result .= "  SEGMENT $row[sortorder] = $row[rank]\n";
    }
    if ($prev) {
      $result .= "ENDGROUP\n";
    }
  } else {
    $result .= "## Divisions\n";
    foreach ($db->query('SELECT name FROM Partitions ORDER BY sortorder') as $part) {
      $result .= "SEGMENT = ".$part['name']."\n";
    }
  }

  $result .= "\n## Awards\n";
  foreach ($db->query('SELECT awardname FROM Awards WHERE classid = 0 AND rankid = 0'
                      .' AND awardtypeid <> 2 AND awardtypeid <> 5') as $award) {
    $result .= "AWARD = ".$award['awardname']."\n";
  }

  $result .= "\n## Settings\n";
  foreach ($db->query('SELECT itemkey, itemvalue FROM RaceInfo ORDER BY raceinfoid') AS $info) {
    // Some values are de facto defaults, meaning the get_raceinfo calls assume
    // a value if the key isn't written.  These are the keys that are explicitly
    // written when the database is initialized.
    if (@array('photos-on-now-racing' => 'head',
               'show-cars-on-deck' => '1',
               'show-racer-photos-rr' => '1',
               'show-car-photos-rr' => '1',
               'upload-videos' => '1')[$info['itemkey']] == $info['itemvalue']) {
      // $result .= "# Default: $info[itemkey] = $info[itemvalue]\n";
      continue;
    }
    // Group formation rule is handled explicitly
    if ($info['itemkey'] == 'group-formation-rule') {
      continue;
    }
    $tag = array_search($info['itemkey'], PREF_BOOL_SETTINGS);
    if ($tag) {
      if (intval($info['itemvalue'])) {
        $result .= "$tag\n";
      } else {
        $result .= "no-$tag\n";
      }
      continue;
    }
    $tag = array_search($info['itemkey'], PREF_STRING_SETTINGS);
    if ($tag) {
      $result .= "$tag = $info[itemvalue]\n";
    } else {
      // $result .= "# $info[itemkey] = $info[itemvalue]\n";
    }
  }

  $first = true;
  foreach ($db->query('SELECT class, ntrophies, rankids, constituents FROM Classes'
                      .' WHERE durable AND (rankids <> \'\' OR constituents <> \'\')') as $row) {
    if ($first) {
      $result .= "\n## Aggregates\n";
      $first = false;
    }
    $result .= "AGGREGATE = $row[class]\n";
    if ($row['ntrophies'] >= 0) {
      $result .= "  NTROPHIES = $row[ntrophies]\n";
    }
    $rankids = explode(',', $row['rankids']);
    if (count($rankids) > 0) {
      foreach ($rankids as $rankid) {
        $rank = read_single_value('SELECT rank FROM Ranks WHERE rankid = :rankid',
                                  array(':rankid' => $rankid));
        $result .= "  CONSTITUENTSUBGROUP = $rank\n";
      }
    } else {
      foreach (explode(',', $row['constituents']) as $classid) {
        $cl = read_single_value('SELECT class FROM Classes WHERE classid = :classid',
                                array(':classid' => $classid));
        $result .= "  CONSTITUENTGROUP = $cl\n";
      }
    }
    $result .= "ENDAGGREGATE\n";
  }

  return $result;
}
?>
