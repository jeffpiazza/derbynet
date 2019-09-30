<?php

require_once('inc/data.inc');
require_once('inc/authorize.inc');
require_once('inc/name-mangler.inc');
require_once('inc/awards.inc');

function export_awards(callable $out) {
  $headings = array();
  $headings[] = "Award Name";
  $headings[] = "Award Type";
  $headings[] = group_label();
  $headings[] = subgroup_label();
  $headings[] = "First Name";
  $headings[] = "Last Name";
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
    $out(array($a['awardname'],
               $a['awardtype'],
               $classes[$a['classid']]['class'],
               $ranks[$a['rankid']]['rank'],
               $a['firstname'],
               $a['lastname'],
               $a['carnumber'],
               $a['carname']
           ));
  }
}

?>