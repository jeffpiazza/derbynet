<?php
require_once('inc/data.inc');
require_once('inc/plural.inc');
require_once('inc/find_or_create_class.inc');
// ** Do not require inc/newracer.inc from here, as newracer.inc requires this file.

$labels = false;
function labels() {
  global $labels;
  // Once per page load, I think.
  if ($labels === false) {
    $supergroup = read_raceinfo('supergroup-label', 'Pack');
    $div_label = read_raceinfo('division-label', 'Den');
    $labels = array(
      'supergroup' => array($supergroup, plural($supergroup)),
      'division'   => array($div_label,  plural($div_label)));

    switch (group_formation_rule()) {
    case 'by-division':
      $labels['group'] = $labels['division'];
      $labels['subgroup'] = array("Subgroup", "Subgroups");
      break;
    case 'one-group':
      $labels['group'] = $labels['supergroup'];
      $labels['subgroup'] = $labels['division'];
      break;
    default:  // custom
      $labels['group'] = array("Group", "Groups");
      $labels['subgroup'] = array("Subgroup", "Subgroups");
    }
  }
  return $labels;
}

function division_label() { return labels()['division'][0]; }
function division_label_lc() { return strtolower(labels()['division'][0]); }
function division_label_pl() { return labels()['division'][1]; }
function division_label_pl_lc() { return strtolower(labels()['division'][1]); }

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
         $rule == 'by-division' ||
         $rule == 'custom';
}

function group_formation_rule() {
  return read_raceinfo('group-formation-rule', 'by-division');
}

function write_group_formation_rule($rule) {
  write_raceinfo('group-formation-rule', $rule);
}

function find_or_create_division($name) {
  global $db;
  $name = trim($name);
  $divisionid = read_single_value('SELECT divisionid FROM Divisions WHERE name = :name',
                                  array(':name' => $name));
  if ($divisionid === false) {
    switch (group_formation_rule()) {
    case 'one-group':
      // one-group: Find or create a rank
      $rankid = find_or_create_rank($name, find_or_create_class('One Group'));
      break;
    default:
      // by-division or custom: Find or create a class
      $classid = find_or_create_class($name);
      $rankid = find_or_create_rank($name, $classid);
    }
    read_single_value('INSERT INTO Divisions(name, rankid, sortorder)'
                      .' VALUES(:name, :rankid,'
                      .'         (SELECT 1 + COALESCE(MAX(sortorder), COUNT(*)) FROM Divisions))',
                      array(':name' => $name, ':rankid' => $rankid));
    // TODO record_event_create_class($classid, $classname);
    $divisionid = read_single_value('SELECT divisionid FROM Divisions WHERE name = :name',
                                    array(':name' => $name));
  }    

  return $divisionid;
}

function clean_up_empty_classes_and_ranks() {
  global $db;
  // Don't delete the last rank from a class that's an antecedent
  $db->exec('DELETE FROM Ranks AS r1'
            .' WHERE rankid NOT IN (SELECT DISTINCT rankid FROM RegistrationInfo)'
            .'   AND (classid NOT IN (SELECT DISTINCT antecedent FROM ConstituentClasses)'
            .'      OR EXISTS(SELECT 1 FROM Ranks AS r2'
            .'                 WHERE r1.classid = r2.classid AND r1.rankid > r2.rankid))');
  $db->exec('DELETE FROM Classes'
            .' WHERE classid NOT IN (SELECT DISTINCT classid FROM RegistrationInfo)'
            .'   AND classid NOT IN (SELECT DISTINCT antecedent FROM ConstituentClasses)'
            .'   AND constituents = \'\''
            .'   AND rankids = \'\'');
}

// Updates sortorder fields for Divisions to agree with ordering of classes and
// ranks.  If members of a division fall in more than class or rank, the
// ordering for the division will be based on the first appearance among classes
// and ranks.
function reorder_divisions() {
  global $db;
  $stmt = $db->prepare('SELECT DISTINCT divisionid FROM '
                       .inner_join('RegistrationInfo', 'Classes',
                                   'RegistrationInfo.classid = Classes.classid',
                                   'Ranks',
                                   'RegistrationInfo.rankid = Ranks.rankid')
                       .' ORDER BY Classes.sortorder, Ranks.sortorder');
  $stmt->execute();

  // A given divisionid may appear more than once in these
  // arrays.  The first appearance wins, by virtue of the fact that the arrays
  // are traversed in reverse order when writing new sortorders
  $divids = array();
  foreach ($stmt as $row) {
    $divids[] = $row['divisionid'];
  }

  $stmt = $db->prepare('UPDATE Divisions SET sortorder = :sortorder WHERE divisionid = :divisionid');
  for ($index = count($divids); $index > 0; --$index) {
    $stmt->execute(array(':sortorder' => $index,
                         ':divisionid' => $divids[$index - 1]));
  }
}

function all_divisions() {
  global $db;
  $stmt = $db->query('SELECT name FROM Divisions ORDER BY sortorder, name');
  if ($stmt) {
    return $stmt->fetchAll(PDO::FETCH_COLUMN, 0);
  }
  return array();
}

?>