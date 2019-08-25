<?php

require_once('inc/schema_version.inc');

// Return value is an array of elements, each having the form:
// {classid:, name:, count:, nrounds_with_results:, ranks:}
//  where ranks is itself an array of {rankid:, name:, count:}
function all_classes() {
  global $db;
  $stmt = $db->prepare('SELECT Classes.classid, class, rankid, rank, '
                       .' (SELECT COUNT(*) FROM RegistrationInfo WHERE RegistrationInfo.classid = Classes.classid) AS nclass,'
                       .' (SELECT COUNT(*) FROM RegistrationInfo WHERE RegistrationInfo.rankid = Ranks.rankid) AS nrank,'
                       .' (SELECT COUNT(*) FROM Rounds'
                       .'     WHERE Rounds.classid = Classes.classid'
                       .'       AND (SELECT COUNT(*) FROM RaceChart'
                       .'             WHERE RaceChart.roundid = Rounds.roundid'
                       .'               AND (finishtime IS NOT NULL OR finishplace IS NOT NULL) > 0)) AS nrounds'
                       .' FROM Classes'
                       .' LEFT JOIN Ranks ON Classes.classid = Ranks.classid'
                       .' ORDER BY '.(schema_version() >= 2
                                      ? 'Classes.sortorder, Ranks.sortorder, '
                                      : '')
                       .'class, rank');
  $stmt->execute(array());
  $classes = [];
  $current = array('classid' => -1);
  foreach ($stmt as $row) {
    if ($row['classid'] != $current['classid']) {
      $classes[] = array('classid' => $row['classid'],
                       'name' => $row['class'],
                       'count' => $row['nclass'],
                       'nrounds_with_results' => $row['nrounds'],
                       'ranks' => []);
      $current = &$classes[count($classes) - 1];
    }
    if ($row['rankid']) {
      $current['ranks'][] = array('rankid' => $row['rankid'],
                                  'name' => $row['rank'],
                                  'count' => $row['nrank']);
    }
  }

  return $classes;
}

?>