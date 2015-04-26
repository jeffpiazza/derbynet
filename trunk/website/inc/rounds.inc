<?php
require_once('inc/schema_version.inc');

/*
Racing groups for different scheduling strategies:

Regular schedules:
  RoundID
  Round
  ClassID
  Class
  GroupID = RoundID
  GroupName = <Class>, Round <Round>

Master schedules:
  GroupID = Round
  GroupName = Round <Round>
 */

function all_rounds() {
  global $db;
  $rounds = array();
  foreach ($db->query('SELECT roundid, round, Classes.classid, class'
					  .' FROM Rounds INNER JOIN Classes'
					  .' ON Rounds.classid = Classes.classid'
					  .' ORDER BY '
                      .(schema_version() >= 2 ? 'Classes.sortorder, ' : '')
                      .'class, round') as $row) {
    $rounds[] = array('roundid' => $row['roundid'],
					  'round' => $row['round'],
					  'classid' => $row['classid'],
					  'class' => $row['class'],
					  'groupid' => $row['roundid'],
					  'groupname' => $row['class'].', Round '.$row['round']);
  }

  return $rounds;
}

function all_racing_groups() {
  global $db;
  if (use_master_sched()) {
    $groups = array();
	foreach ($db->query('SELECT DISTINCT round'
						.' FROM Rounds'
						.' ORDER BY round') as $row) {
      $groups[] = array('round' => $row['round'],
						'groupid' => $row['round'],
						'groupname' => 'Round '.$row['round']);
    }

    return $groups;
  } else {
    return all_rounds();
  }
}

?>