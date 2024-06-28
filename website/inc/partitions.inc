<?php
require_once('inc/data.inc');
require_once('inc/plural.inc');
require_once('inc/class_add_or_delete.inc');
require_once('inc/rank_add_or_delete.inc');
// ** Do not require inc/newracer.inc from here, as newracer.inc requires this file.

define('DEFAULT_PARTITION_NAME', 'Default');

$labels = false;
function labels() {
  global $labels;
  // Once per page load, I think.
  if (!isset($labels) || $labels === false) {
    $supergroup = read_raceinfo('supergroup-label', 'Pack');
    $div_label = read_raceinfo('partition-label', 'Den');
    $labels = array(
      'supergroup' => array($supergroup, plural($supergroup)),
      'partition'  => array($div_label,  plural($div_label)));

    switch (group_formation_rule()) {
    case 'by-partition':
      $labels['group'] = $labels['partition'];
      $labels['subgroup'] = array("Subgroup", "Subgroups");
      break;
    case 'one-group':
      $labels['group'] = array("Group", "Groups");
      $labels['subgroup'] = $labels['partition'];
      break;
    default:  // custom
      $labels['group'] = array("Group", "Groups");
      $labels['subgroup'] = array("Subgroup", "Subgroups");
    }
  }
  return $labels;
}

function partition_label() { return labels()['partition'][0]; }
function partition_label_lc() { return strtolower(labels()['partition'][0]); }
function partition_label_pl() { return labels()['partition'][1]; }
function partition_label_pl_lc() { return strtolower(labels()['partition'][1]); }

function supergroup_label() { return labels()['supergroup'][0]; }
function supergroup_label_lc() { return strtolower(labels()['supergroup'][0]); }
function supergroup_label_pl() { return labels()['supergroup'][1]; }
function supergroup_label_pl_lc() { return strtolower(labels()['supergroup'][1]); }

function group_label() { return labels()['group'][0]; }
function group_label_lc() { return strtolower(labels()['group'][0]); }
function group_label_pl() { return labels()['group'][1]; }
function group_label_pl_lc() { return strtolower(labels()['group'][1]); }

function subgroup_label() { return labels()['subgroup'][0]; }
function subgroup_label_lc() { return strtolower(labels()['subgroup'][0]); }
function subgroup_label_pl() { return labels()['subgroup'][1]; }
function subgroup_label_pl_lc() { return strtolower(labels()['subgroup'][1]); }

function is_allowed_group_formation_rule($rule) {
  return $rule == 'one-group' ||
         $rule == 'by-partition' ||
         $rule == 'custom';
}

function group_formation_rule() {
  return read_raceinfo('group-formation-rule', 'by-partition');
}

function write_group_formation_rule($rule) {
  write_raceinfo('group-formation-rule', $rule);
}

function find_or_create_partition($name) {
  global $db;
  $name = trim($name);
  $partitionid = read_single_value('SELECT partitionid FROM Partitions WHERE name = :name',
                                  array(':name' => $name));
  if ($partitionid === false) {
    switch (group_formation_rule()) {
    case 'one-group':
      // one-group: Find or create a rank
      $rankid = find_or_create_rank($name,
                                    read_single_value('SELECT MIN(classid) FROM Classes'),
                                    /*with_partition=*/false);
      break;
    default:
      // by-partition or custom: Find or create a class
      $classid = find_or_create_class($name);
      $rankid = find_or_create_rank($name, $classid, /*with_partition=*/false);
    }
    read_single_value('INSERT INTO Partitions(name, rankid, sortorder)'
                      .' VALUES(:name, :rankid,'
                      .'         (SELECT 1 + COALESCE(MAX(sortorder), COUNT(*)) FROM Partitions))',
                      array(':name' => $name, ':rankid' => $rankid));
    // TODO record_event EVENT_CLASS_ADDED $classid, $classname
    $partitionid = read_single_value('SELECT partitionid FROM Partitions WHERE name = :name',
                                    array(':name' => $name));
    record_action(array('action' => 'partition.*create',
                        'partitionid' => $partitionid,
                        'name' => $name,
                        'rankid' => $rankid));
  }    

  return $partitionid;
}

function create_partition_in_existing_rank($name, $rankid) {
  read_single_value('INSERT INTO Partitions(name, rankid, sortorder)'
                    .' VALUES(:name, :rankid,'
                    .'        (SELECT 1 + COALESCE(MAX(sortorder), COUNT(*)) FROM Partitions))',
                    array(':name' => $name,
                          ':rankid' => $rankid));
  record_action(array('action' => 'partition.*create',
                      'partitionid' => read_single_value('SELECT partitionid FROM Partitions WHERE name = :name',
                                                         array(':name' => $name)),
                      'name' => $name,
                      'rankid' => $rankid));
}

function clean_up_empty_classes_and_ranks() {
  global $db;

  // Clean up after partition rule change, especially after moving a partition
  // under custom rule.
  //
  // To minimize disruption, we avoid deleting classes that are antecedent to
  // some aggregate class.  Since every organic class needs at least one
  // associated rank, we avoid deleting the smallest rankid for a class that
  // won't be deleted.

  // Organic classes include ranks for which a partition exists.
  // Aggregate classes have either non-empty constituents field or non-empty rankids field.

  /* Some older SQLite implementations don't support this:
  $db->exec('DELETE FROM Ranks AS r1'
            .' WHERE rankid NOT IN (SELECT DISTINCT rankid FROM Partitions)'
            .'   AND (classid NOT IN (SELECT DISTINCT antecedent FROM ConstituentClasses)'
            .'      OR rankid > (SELECT MIN(rankid) FROM Ranks AS r2 WHERE r1.classid = r2.classid))');
  */
  $unused_ranks = $db->query('SELECT rankid, classid FROM Ranks WHERE rankid NOT IN (SELECT DISTINCT rankid FROM Partitions)')
                     ->fetchAll(PDO::FETCH_ASSOC);
  foreach ($unused_ranks AS $rank) {
    if (read_single_value('SELECT COUNT(*) FROM ConstituentClasses WHERE antecedent = :classid',
                          array(':classid' => $rank['classid'])) > 0) {
      if (read_single_value('SELECT MIN(rankid) FROM Ranks WHERE classid = :classid',
                            array(':classid' => $rank['classid'])) == $rank['rankid']) {
        // Don't delete the first rank of a class that's an antecedent to an aggregate
        continue;
      }
    }
    $stmt = $db->prepare('DELETE FROM Ranks WHERE rankid = :rankid');
    $stmt->execute(array(':rankid' => $rank['rankid']));
  }

  if (group_formation_rule() != 'one-group') {
    $db->exec('DELETE FROM Classes'
              .' WHERE classid NOT IN (SELECT DISTINCT classid FROM Ranks)'
              .'   AND classid NOT IN (SELECT DISTINCT antecedent FROM ConstituentClasses)'
              .'   AND constituents = \'\''
              .'   AND rankids = \'\'');
  }
  $db->exec('DELETE FROM RaceInfo WHERE itemkey = \'full-field-calc\''
            .' AND itemvalue NOT IN (SELECT classid FROM Classes)');
  
  $db->exec('DELETE FROM Rounds'
            .' WHERE classid NOT IN (SELECT classid FROM Classes)');
  $db->exec('DELETE FROM Roster'
            .' WHERE roundid NOT IN (SELECT roundid FROM Rounds)');
  $db->exec('DELETE FROM RaceChart'
            .' WHERE roundid NOT IN (SELECT roundid FROM Rounds)');
  $db->exec('DELETE FROM Playlist'
            .' WHERE classid NOT IN (SELECT classid FROM Classes)');

  $db->exec('DELETE FROM Awards'
            .' WHERE classid IS NOT NULL AND classid <> 0'
            .'  AND classid NOT IN (SELECT classid FROM Classes)');
  $db->exec('DELETE FROM Awards'
            .' WHERE rankid IS NOT NULL AND rankid <> 0'
            .'  AND rankid NOT IN (SELECT rankid FROM Ranks)');
}

// Updates sortorder fields for Partitions to agree with ordering of classes and
// ranks.
function reorder_partitions() {
  global $db;

  $stmt = $db->prepare('SELECT partitionid FROM '
                       .inner_join('Partitions', 'Ranks',
                                   'Partitions.rankid = Ranks.rankid',
                                   'Classes',
                                   'Classes.classid = Ranks.classid')
                       .' ORDER BY Classes.sortorder, Ranks.sortorder');
  $stmt->execute();

  // A given partitionid may appear more than once in these
  // arrays.  The first appearance wins, by virtue of the fact that the arrays
  // are traversed in reverse order when writing new sortorders
  $divids = array();
  foreach ($stmt as $row) {
    $divids[] = $row['partitionid'];
  }

  $stmt = $db->prepare('UPDATE Partitions SET sortorder = :sortorder WHERE partitionid = :partitionid');
  for ($index = count($divids); $index > 0; --$index) {
    $stmt->execute(array(':sortorder' => $index,
                         ':partitionid' => $divids[$index - 1]));
  }
}

function all_partitions() {
  global $db;
  $stmt = $db->query('SELECT name FROM Partitions ORDER BY sortorder, name');
  if ($stmt) {
    return $stmt->fetchAll(PDO::FETCH_COLUMN, 0);
  }
  return array();
}

function record_action_partition_structure() {
  global $db;
  $ranks = $db->query('SELECT rankid, classid, roundid'
                      .' FROM Ranks'
                      .' INNER JOIN Classes USING (classid)'
                      .' INNER JOIN Rounds USING (classid)'
                      .' WHERE round = 1'  // The only rounds that exist all have round = 1
                      .' AND NOT EXISTS(SELECT 1 FROM Partitions WHERE rankid = Ranks.rankid)'
                     .' ORDER BY rankid')
                 ->fetchAll(PDO::FETCH_ASSOC);
  $partition_map = $db->query('SELECT partitionid, classid, rankid, roundid'
                              .' FROM Partitions'
                              .' INNER JOIN Ranks USING (rankid)'
                              .' INNER JOIN Classes USING (classid)'
                              .' INNER JOIN Rounds USING (classid)'
                              .' WHERE round = 1'  // The only rounds that exist all have round = 1
                     .' ORDER BY partitionid')
                 ->fetchAll(PDO::FETCH_ASSOC);
  if (group_formation_rule() == 'one-group') {
    record_action(array('action' => 'partition.*structure',
                        'class' => 'All Racers',
                        'partitions' => $partition_map,
                        'ranks' => $ranks));
  } else {
    record_action(array('action' => 'partition.*structure',
                        'partitions' => $partition_map,
                        'ranks' => $ranks));
  }
}

?>
