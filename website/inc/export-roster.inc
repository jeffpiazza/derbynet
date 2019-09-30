<?php

function export_roster(callable $out) {
  global $db;
  $headings = array();
  $headings[] = "First Name";
  $headings[] = "Last Name";
  $headings[] = "Car Number";
  $headings[] = "Car Name";
  $headings[] = group_label();
  $headings[] = subgroup_label();
  $headings[] = 'Passed?';
  $headings[] = 'Excluded?';

  $out($headings);

// TODO Ordering options:
//    By lastname, firstname
//    By car number
//    By class, then lastname,firstname
//    By class, then car number
//    By class then rank then lastname, firstname
//    By class then rank then car number

  $stmt = $db->query('SELECT firstname, lastname, carnumber, carname, '
                     .' passedinspection, exclude,'
                     .' Classes.class as classname,'
                     .' Ranks.rank as rankname'
                     .' FROM '.inner_join('RegistrationInfo',
                                          'Classes', 'RegistrationInfo.classid = Classes.classid',
                                          'Ranks', 'RegistrationInfo.rankid = Ranks.rankid')
                     // TODO passed?  excluded?
                     .' ORDER BY '.'lastname, firstname'
    );
  if ($stmt === false) {
    // TODO
  }
  foreach ($stmt as $row) {
    $out(array($row['firstname'],
               $row['lastname'],
               $row['carnumber'],
               $row['carname'],
               $row['classname'],
               $row['rankname'],
               $row['passedinspection'],
               $row['exclude']));
  }
}

?>