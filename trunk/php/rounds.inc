<?php

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
  global $conn;
  $sql = 'SELECT RoundID, Round, classes.ClassID, Class'
    .' FROM rounds INNER JOIN classes'
    .' ON rounds.ClassID = classes.ClassID'
    .' ORDER BY Class, Round';
  $rs = odbc_exec($conn, $sql);
  $rounds = array();
  while (odbc_fetch_row($rs)) {
    $rounds[] = array('RoundID' => odbc_result($rs, 'RoundID'),
		      'Round' => odbc_result($rs, 'Round'),
		      'ClassID' => odbc_result($rs, 'ClassID'),
		      'Class' => odbc_result($rs, 'Class'),
		      'GroupID' => odbc_result($rs, 'RoundID'),
		      'GroupName' => odbc_result($rs, 'Class').', Round '.odbc_result($rs, 'Round'));
  }

  return $rounds;
}

function all_racing_groups() {
  global $conn;
  if (use_master_sched()) {
    $sql = 'SELECT distinct Round'
      .' FROM rounds'
      .' ORDER BY Round';
    $rs = odbc_exec($conn, $sql);
    $groups = array();
    while (odbc_fetch_row($rs)) {
      $groups[] = array('Round' => odbc_result($rs, 'Round'),
			'GroupID' => odbc_result($rs, 'Round'),
			'GroupName' => 'Round '.odbc_result($rs, 'Round'));
    }

    return $groups;
  } else {
    return all_rounds();
  }
}

?>