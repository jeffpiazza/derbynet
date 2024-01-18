<?php

require_once('inc/partitions.inc');
require_once('inc/name-mangler.inc');

function export_roster(callable $out) {
  global $db;

  $use_groups = use_groups();
  $use_subgroups = use_subgroups();
  $name_style = read_name_style();
  $show_racerid = isset($_GET['racerid']);

  $headings = array();
  if ($show_racerid) {
    $headings[] = "RacerID";
  }
  if ($name_style == FULL_NAME) {
    $headings[] = "First Name";
    $headings[] = "Last Name";
  } else {
    $headings[] = "Name";
  }
  $headings[] = "Car Number";
  $headings[] = "Car Name";
  if ($use_groups) {
    $headings[] = group_label();
  }
  if ($use_subgroups) {
    $headings[] = subgroup_label();
  }
  $headings[] = 'Excluded?';

  $out($headings);

// TODO Ordering options:
//    By lastname, firstname
//    By car number
//    By class, then lastname,firstname
//    By class, then car number
//    By class then rank then lastname, firstname
//    By class then rank then car number

  $stmt = $db->query('SELECT racerid, firstname, lastname, carnumber, carname, '
                     .' passedinspection, exclude,'
                     .' Classes.class as classname,'
                     .' Ranks.rank as rankname'
                     .' FROM '.inner_join('RegistrationInfo',
                                          'Classes', 'RegistrationInfo.classid = Classes.classid',
                                          'Ranks', 'RegistrationInfo.rankid = Ranks.rankid')
                     .' WHERE passedinspection = 1'
                     .' ORDER BY '.'lastname, firstname'
    );
  if ($stmt === false) {
    // TODO
  }
  foreach ($stmt as $row) {
    $line = array();
    if ($show_racerid) {
      $line[] = $row['racerid'];
    }
    if ($name_style == FULL_NAME) {
      $line[] = $row['firstname'];
      $line[] = $row['lastname'];
    } else {
      $line[] = mangled_name($row, $name_style);
    }

    $line[] = $row['carnumber'];
    $line[] = $row['carname'];

    if ($use_groups) {
      $line[] = $row['classname'];
    }
    if ($use_subgroups) {
      $line[] = $row['rankname'];
    }
    $line[] = $row['exclude'];

    $out($line);
  }
}

?>
