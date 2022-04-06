<?php

require_once('inc/data.inc');
require_once('inc/partitions.inc');
require_once('inc/classes.inc');
require_once('inc/authorize.inc');
require_once('inc/name-mangler.inc');
require_once('inc/awards.inc');

function export_awards(callable $out) {
  $name_style = read_name_style();
  $use_groups = use_groups();
  $use_subgroups = use_subgroups();

  $headings = array();
  $headings[] = "Award Name";
  $headings[] = "Award Type";
  if ($use_groups) {
    $headings[] = group_label();
  }
  if ($use_subgroups) {
    $headings[] = subgroup_label();
  }
  if ($name_style == FULL_NAME) {
    $headings[] = "First Name";
    $headings[] = "Last Name";
  } else {
    $headings[] = "Name";
  }
  $headings[] = "Car Number";
  $headings[] = "Car Name";

  $out($headings);

// TODO Group awards by award class, award rank; order by class sort order, rank sort order.
// TODO Awards without class or rank.

  list($classes, $classseq, $ranks, $rankseq) = classes_and_ranks();

  $awards = array();
  $reserved_overall = add_speed_awards($awards);
  $awards = array_merge($awards, all_awards(/* include_ad_hoc */ true, $reserved_overall));

  foreach ($awards as $a) {
    // TODO
    if ($a['classid'] == 0) $a['classid'] = '';
    if ($a['rankid'] == 0) $a['rankid'] = '';
    $line = array($a['awardname'],
                  $a['awardtype']);
    if ($use_groups) {
      $line[] = @$classes[$a['classid']]['class'];
    }
    if ($use_subgroups) {
      $line[] = @$ranks[$a['rankid']]['rank'];
    }
    if ($name_style == FULL_NAME) {
      $line[] = $a['firstname'];
      $line[] = $a['lastname'];
    } else {
      $line[] = mangled_name($a, $name_style);
    }
    $line[] = $a['carnumber'];
    $line[] = $a['carname'];

    $out($line);
  }
}

?>