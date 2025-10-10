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
// ## Obsolete: SEGMENT = (partition name)
//
// GROUP = (group name) starts a context that extends to the next ENDGROUP.
// Within a group, SUBGROUPs, AWARDs, or NTROPHIES may appear.
//
// SIMPLE GROUP = (group name) defines a group with a single subgroup with the
// same name and no customizations.
//
// SUBGROUP = (subgroup name) starts a context that extends to the next
// ENDSUBGROUP.  Subgroups must appear only within a group.  AWARDs or
// NTROPHIES may appear within a subgroup.
//
// SIMPLE SUBGRUP = (subgroup name) defines a subgroup within a group that
// includes no customizations.
//
// AGGREGATE = (aggregate group name) starts a context that extends to the next
// ENDAGGREGATE.  Within an aggregate context, CONSTITUENT GROUP or
// CONSTITUENT SUBGROUPs may appear, but not a mix of the two.

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

    if ($line_tag == 'SUBGROUP' && isset($val)) {
      if (is_null($curr_group)) {
        json_failure('subgroup-needs-a-group',
                     "At line $lineno, SUBGROUP $val without an enclosing GROUP");
        return;
      }
      if (!is_null($curr_subgroup)) {
        json_failure('subgroup-within-subgroup',
                     "At line $lineno, SUBGROUP $val within another subgroup");
        return;
      }
      $curr_subgroup = find_or_create_rank($val, $curr_group, /*with_partition=*/false);
      create_partition_in_existing_rank($val, $curr_subgroup);
      array_push($stack, array('SUBGROUP', $val));
      continue;
    }
    if ($line_tag == 'ENDSUBGROUP') {
      if (is_null($curr_subgroup)) {
        json_failure('endsubgroup-no-subgroup', "At line $lineno, ENDSUBGROUP without SUBGROUP");
        return;
      }
      $curr_subgroup = null;
      if ($stack[0][0] == 'SUBGROUP') {
        array_pop($stack);
      }
      continue;
    }

    if ($line_tag == 'SIMPLE' && isset($val)) {
      if ($arg == 'GROUP') {
        if (!is_null($curr_group) || !is_null($curr_aggregate)) {
          json_failure('group-within-group', "At line $lineno, GROUP $val within another group or aggregate");
          return;
        }

        // TODO In one-group, this should be disallowed?

        $classid = find_or_create_class($val);
        $rankid = find_or_create_rank($val, $classid, /*with_partition=*/false);
        create_partition_in_existing_rank($val, $rankid);

        continue;
      } else if ($arg = 'SUBGROUP') {
        if (is_null($curr_group)) {
          json_failure('subgroup-needs-a-group',
                       "At line $lineno, SUBGROUP $val without an enclosing GROUP");
          return;
        }
        $rankid = find_or_create_rank($val, $curr_group, /*with_partition=*/false);
        create_partition_in_existing_rank($val, $rankid);
        continue;
      } else {
        json_failure('not-simple', "At line $lineno, SIMPLE $arg not recognized.");
        return;
      }
    }
    if ($line_tag == 'SEGMENT' && isset($val)) {
      $partitionid = read_single_value('SELECT partitionid FROM Partitions WHERE name = :val',
                                       array(':val' => $val));
      if ($partitionid !== false) {
        json_failure('division-exists', "At line $lineno , SEGMENT $val already exists.");
        return;
      }
      if (!is_null($curr_subgroup)) {
        // This would support having multiple partitions within a single
        // subgroup, something we don't actually allow.
        create_partition_in_existing_rank($val, $curr_subgroup);
      } else if (!is_null($curr_group)) {
        $rankid = find_or_create_rank($val, $curr_group, /*with_partition=*/false);
        create_partition_in_existing_rank($val, $rankid);
      } else {
        if (group_formation_rule() == 'one-group' &&
            read_single_value('SELECT COALESCE(MIN(classid), -1) FROM Classes')) {
          find_or_create_class('All Racers');
        }
        // For by-partition or custom, this will create a new class and rank
        // corresponding to the new partition.  (We already checked that there
        // isn't an existing partition with this name.)
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
        read_single_value('UPDATE Ranks SET ntrophies = :ntrophies WHERE rankid = :rankid',
                          array(':ntrophies' => $val,
                                ':rankid' => $curr_subgroup));
        continue;
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
    if ($line_tag == 'CONSTITUENTGROUP') {
      $line_tag = 'CONSTITUENT';
      $arg = 'GROUP';
      // Fall through
    }
    if ($line_tag == 'CONSTITUENTSUBGROUP') {
      $line_tag = 'CONSTITUENT';
      $arg = 'SUBGROUP';
      // Fall through
    }
    if ($line_tag ==  'CONSTITUENT' && $arg == 'GROUP' && isset($val)) {
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
    if ($line_tag ==  'CONSTITUENT' && $arg == 'SUBGROUP' && isset($val)) {
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

class PreferencePeepholer {
  private $result;

  private $group;
  private $use_simple_group;
  private $subgroup;
  private $use_simple_subgroup;
  private $matching_subgroup_seen;
  private $depth = 0;

  // TODO Make this private
  public function write_raw($str) {
    $this->result .= str_repeat('  ', $this->depth).$str;
  }
  public function write($str) {
    $this->flush_group();
    $this->flush_subgroup();
    $this->write_raw($str);
  }

  public function debug() {
    $this->write_raw("#### DEBUG group:$this->group simple:$this->use_simple_group matched:$this->matching_subgroup_seen\n");
    $this->write_raw("#### DEBUG subgroup:$this->subgroup simple:$this->use_simple_subgroup\n");
  }

  private function flush_group() {
    if ($this->use_simple_group) {
      $this->write_raw("GROUP = $this->group\n");
      ++$this->depth;
      if ($this->matching_subgroup_seen) {
        $this->write_raw("SIMPLE SUBGROUP = $this->group\n");
      }
      $this->group = '';
      $this->use_simple_group = false;
    }
  }

  private function flush_subgroup() {
    if ($this->use_simple_subgroup) {
      $this->flush_group();
      $this->write_raw("SUBGROUP = $this->subgroup\n");
      ++$this->depth;
      $this->subgroup = '';
      $this->use_simple_subgroup = false;
    }
  }

  public function begin_group($group) {
    $this->group = $group;
    $this->use_simple_group = true;
    $this->subgroup = '';
    $this->matching_subgroup_seen = false;
  }
  public function end_group() {
    if ($this->use_simple_group) {
      if ($this->matching_subgroup_seen) {
        $this->write_raw("SIMPLE GROUP = $this->group\n");
      } else {
        $this->write_raw("GROUP = $this->group\n");
        $this->write_raw("### Empty group!\n");
        $this->write_raw("ENDGROUP\n");
      }
    } else {
      --$this->depth;
      $this->write_raw("ENDGROUP\n");
    }
    $this->group = '';
    $this->subgroup = '';
  }

  public function begin_subgroup($subgroup) {
    $this->subgroup = $subgroup;
    $this->use_simple_subgroup = true;
  }

  public function end_subgroup() {
    if ($this->use_simple_subgroup) {
      if ($this->use_simple_group) {
        if ($this->subgroup == $this->group) {
          $this->matching_subgroup_seen = true;
        } else {
          $this->flush_group();
          $this->write_raw("SIMPLE SUBGROUP = $this->subgroup\n");
        }
      } else {
        $this->write_raw("SIMPLE SUBGROUP = $this->subgroup\n");
      }
    } else {
      --$this->depth;
      $this->write_raw("ENDSUBGROUP\n");
    }
    $this->use_simple_subgroup = false;  // Otherwise it looks like we're still in the just-closed subgroup
  }

  public function to_string() { return $this->result; }
}

// Returns a string suitable for writing to a preferences file.
function dump_preferences() {
  global $db;
  $result = new PreferencePeepholer();
  $result->write_raw("# Preferences written ".date("Y-m-d")."\n\n");
  $group_rule = group_formation_rule();
  if ($group_rule != 'by-partition') {
    $result->write_raw("group-formation-rule = $group_rule\n");
  }

  $result->write_raw("\n");
  $prev = '';
  foreach ($db->query('SELECT class, classid, rank, rankid,'
                      .'      Classes.ntrophies, Ranks.ntrophies AS rank_ntrophies,'
                      .'      Partitions.sortorder AS sortorder'
                      .' FROM Ranks JOIN Classes USING (classid)'
                      .'            JOIN Partitions USING (rankid)'
                      .' ORDER BY Classes.sortorder, Ranks.sortorder') as $row) {
    if ($row['classid'] <> $prev) {
      if ($prev) {
        $result->end_group();
      }
      $result->begin_group($row['class']);
      $prev = $row['classid'];

      if ($row['ntrophies'] >= 0) {
        $result->write("NTROPHIES = $row[ntrophies]\n");
      }
      foreach ($db->query("SELECT awardname FROM Awards WHERE classid = $row[classid] AND rankid = 0"
                          ." AND awardtypeid <> 2 AND awardtypeid <> 5") as $award) {
        $result->write("AWARD = $award[awardname]\n");
      }
    }
    $result->begin_subgroup($row['rank']);
    if ($row['rank_ntrophies'] >= 0) {
      $result->write("NTROPHIES = $row[rank_ntrophies]\n");
    }
    foreach ($db->query("SELECT awardname FROM Awards WHERE rankid = $row[rankid]"
                        ." AND awardtypeid <> 2 AND awardtypeid <> 5") as $award) {
      $result->write("AWARD = $award[awardname]\n");
    }
    $result->end_subgroup();
  }
  if ($prev) {
    $result->end_group();
  }

  $first = true;
  foreach ($db->query('SELECT awardname FROM Awards WHERE classid = 0 AND rankid = 0'
                      .' AND awardtypeid <> 2 AND awardtypeid <> 5') as $award) {
    if ($first) {
      $result->write_raw("\n");
      $first = false;
    }
    $result->write_raw("AWARD = ".$award['awardname']."\n");
  }

  $first = true;
  foreach ($db->query('SELECT itemkey, itemvalue FROM RaceInfo ORDER BY raceinfoid') AS $info) {
    if (@array(
          // These are the keys that are explicitly
          // written when the database is initialized.
          'photos-on-now-racing' => 'head',
          'show-cars-on-deck' => '1',
          'show-racer-photos-rr' => '1',
          'show-car-photos-rr' => '1',
          'upload-videos' => '1',
          // Some values are de facto defaults, meaning the get_raceinfo calls assume
          // a value if the key isn't written.
          'lane-colors' => '',
        )[$info['itemkey']] == $info['itemvalue']) {
      // $result->write_raw("# Default: $info[itemkey] = $info[itemvalue]\n");
      continue;
    }
    
    // Group formation rule is handled explicitly
    if ($info['itemkey'] == 'group-formation-rule') {
      continue;
    }
    if ($first) {
      $result->write_raw("\n");
      $first = false;
    }
    $tag = array_search($info['itemkey'], PREF_BOOL_SETTINGS);
    if ($tag) {
      if (intval($info['itemvalue'])) {
        $result->write_raw("$tag\n");
      } else {
        $result->write_raw("no-$tag\n");
      }
      continue;
    }
    $tag = array_search($info['itemkey'], PREF_STRING_SETTINGS);
    if ($tag) {
      $result->write_raw("$tag = $info[itemvalue]\n");
    } else {
      // $result->write_raw("# $info[itemkey] = $info[itemvalue]\n");
    }
  }

  $first = true;
  foreach ($db->query('SELECT class, classid, ntrophies, rankids, constituents FROM Classes'
                      .' WHERE durable AND (rankids <> \'\' OR constituents <> \'\')') as $row) {
    if ($first) {
      $result->write_raw("\n");
      $first = false;
    }
    $result->write_raw("AGGREGATE = $row[class]\n");

    if ($row['ntrophies'] >= 0) {
      $result->write_raw("  NTROPHIES = $row[ntrophies]\n");
    }
    foreach ($db->query("SELECT awardname FROM Awards WHERE classid = $row[classid] AND rowid = 0"
                        ." AND awardtypeid <> 2 AND awardtypeid <> 5") as $award) {
      $result->write("AWARD = $award[awardname]\n");
    }

    if (!empty($row['rankids'])) {
      $rankids = explode(',', $row['rankids']);
      foreach ($rankids as $rankid) {
        $rank = read_single_value('SELECT rank FROM Ranks WHERE rankid = :rankid',
                                  array(':rankid' => $rankid));
        $result->write_raw("  CONSTITUENT SUBGROUP = $rank\n");
      }
    } else {
      foreach (explode(',', $row['constituents']) as $classid) {
        $cl = read_single_value('SELECT class FROM Classes WHERE classid = :classid',
                                array(':classid' => $classid));
        $result->write_raw("  CONSTITUENT GROUP = $cl\n");
      }
    }
    $result->write_raw("ENDAGGREGATE\n");
  }

  return $result->to_string();
}
?>
