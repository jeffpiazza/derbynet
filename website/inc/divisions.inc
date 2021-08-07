<?php
require_once('inc/data.inc');
require_once('inc/plural.inc');

function division_label() { return "Den"; }
function division_label_lc() { return strtolower(division_label()); }

function division_label_plural() { return plural(division_label()); }
function division_label_plural_lc() { return strtolower(division_label_plural()); }

function subdivision_label() { return "Subdivision"; }
function subdivision_label_lc() { return strtolower(subdivision_label()); }

function subdivision_label_plural() { return plural(subdivision_label()); }
function subdivision_label_plural_lc() { return strtolower(subdivision_label_plural()); }

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
    read_single_value('INSERT INTO Divisions(name, sortorder)'
                      .' VALUES(:name, (SELECT 1 + COALESCE(MAX(sortorder), COUNT(*)) FROM Divisions))',
                      array(':name' => $name));
    // TODO record_event_create_class($classid, $classname);
    $divisionid = read_single_value('SELECT divisionid FROM Divisions WHERE name = :name',
                                    array(':name' => $name));
  }    

  return $divisionid;
}

function find_or_create_subdivision($name, $divid) {
  global $db;
  $name = trim($name);
  $subdivisionid = read_single_value('SELECT subdivisionid FROM Subdivisions'
                                     .' WHERE name = :name AND superdiv = :superdiv',
                                     array(':name' => $name,
                                           ':superdiv' => $divid));
  if ($subdivisionid === false) {
    read_single_value('INSERT INTO Subdivisions(name, superdiv, sortorder)'
                      .' VALUES(:name, :superdiv,'
                      .'     (SELECT 1 + COALESCE(MAX(sortorder), COUNT(*)) FROM Subdivisions'
                      .'          WHERE superdiv = :superdiv2))',
                      array(':name' => $name,
                            ':superdiv' => $divid,
                            ':superdiv2' => $divid));
    // TODO record_event_create_(...)
    $subdivisionid = read_single_value('SELECT subdivisionid FROM Subdivisions'
                                       .' WHERE name = :name AND superdiv = :superdiv',
                                       array(':name' => $name,
                                             ':superdiv' => $divid));
  }    

  return $subdivisionid;
}

// If any racer has a non-null subdivisionid, then ALL racers should have non-null subdivisionid.
function fix_up_missing_subdivisions() {
  global $db;
  if (read_single_value('SELECT COUNT(*) FROM Subdivisions') > 0 &&
      read_single_value('SELECT COUNT(*) FROM RegistrationInfo WHERE subdivisionid IS NULL') > 0) {
    // If there are any subdivisions, then all racers should be in one.
    $stmt = $db->prepare('SELECT divisionid, name FROM Divisions'
                         .' WHERE NOT EXISTS(SELECT 1 FROM Subdivisions WHERE superdiv = divisionid)');
    $stmt->execute();
    foreach ($stmt as $row) {
      find_or_create_subdivision($row['name'], $row['divisionid']);
    }
    $db->exec('UPDATE RegistrationInfo SET subdivisionid = '
              .'    (SELECT MIN(subdivisionid) FROM Subdivisions WHERE superdiv = divisionid)'
              .'    WHERE subdivisionid IS NULL');
  }
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

// Updates sortorder fields for Divisions and Subdivisions to agree with
// ordering of classes and ranks.  If members of a division or subdivision fall
// in more than class or rank, the ordering for the division or subdivision will
// be based on the first appearance among classes and ranks.
function reorder_divisions() {
  global $db;
  // select distinct rankid, classid, subdivisionid from registrationInfo
  // select distinct rankid, classid, divisionid from registrationInfo join classes join ranks order by classes.sortorder, ranks.sortorder
  $stmt = $db->prepare('SELECT DISTINCT divisionid, subdivisionid FROM '
                       .inner_join('RegistrationInfo', 'Classes',
                                   'RegistrationInfo.classid = Classes.classid',
                                   'Ranks',
                                   'RegistrationInfo.rankid = Ranks.rankid')
                       .' ORDER BY Classes.sortorder, Ranks.sortorder');
  $stmt->execute();

  // A given divisionid or subdivisionid may appear more than once in these
  // arrays.  The first appearance wins, by virtue of the fact that the arrays
  // are traversed in reverse order when writing new sortorders
  $divids = array();
  $subdivids = array();
  foreach ($stmt as $row) {
    $divids[] = $row['divisionid'];
    if (isset($row['subdivisionid'])) {
      $subdivids[] = $row['subdivisionid'];
    }
  }

  $stmt = $db->prepare('UPDATE Divisions SET sortorder = :sortorder WHERE divisionid = :divisionid');
  for ($index = count($divids); $index > 0; --$index) {
    $stmt->execute(array(':sortorder' => $index,
                         ':divisionid' => $divids[$index - 1]));
  }

  // Subdivision.sortorder will be an absolute ordering, although it only needs
  // to be relative to the containing division.
  if (count($subdivids) > 0) {
    $stmt = $db->prepare('UPDATE Subdivisions SET sortorder = :sortorder WHERE subdivisionid = :subdivisionid');
    for ($index = count($subdivids); $index > 0; --$index) {
      $stmt->execute(array(':sortorder' => $index,
                           ':subdivisionid' => $subdivids[$index - 1]));
    }
  }
}

?>