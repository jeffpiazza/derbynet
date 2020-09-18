<?php

require_once('inc/schema_version.inc');

// Return value is an array of elements, each having the form:
// {classid:, name:, count:, nrounds_with_results:, ranks:}
//  where ranks is itself an array of {rankid:, name:, count:}
function all_classes() {
  global $db;
  $stmt = $db->prepare('SELECT Classes.classid, class, rankid, rank, '
                       .(schema_version() >= 3 ? 'ntrophies, ' : '-1 as ntrophies, ')
                       .(schema_version() >= 3 ? 'constituents, ' : '\'\' as constituents, ')
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
                         'ntrophies' => $row['ntrophies'],
                         'constituents' => $row['constituents'],
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

// list($classes, $classseq, $ranks, $rankseq) = classes_and_ranks();
//   $classes:  classid => {'class', 'constituents', 'ntrophies', 'seq'}
//   $classseq: ordered classids
//   $ranks:    rankid => {'class', 'classid', 'rank', 'rankid', 'seq'}
//   $rankseq:  ordered rankids
//
// Note that (of course) $ranks and $rankseq won't account for aggregate
// classes, which don't have any ranks.
function classes_and_ranks() {
  global $db;

  $classes = array();
  $classseq = array();
  $ranks = array();
  $rankseq = array();
  foreach ($db->query('SELECT Classes.classid, class, rankid, rank,'
                      .(schema_version() >= 3
                        ? 'constituents, ntrophies'
                        : '\'\' AS constituents, -1 AS ntrophies')
                      .' FROM Classes'
                      .' LEFT JOIN Ranks ON Classes.classid = Ranks.classid'
                      .' ORDER BY '.(schema_version() >= 2
                                     ? 'Classes.sortorder, Ranks.sortorder, '
                                     : '')
                      .'class, rank') as $row) {
    if (!isset($classes[$row['classid']])) {
      $classseq[] = $row['classid'];
      $classes[$row['classid']] = array('class' => $row['class'],
                                        'constituents' => array_filter(explode(',', $row['constituents']),
                                                                       'strlen'),
                                        'ntrophies' => $row['ntrophies'],
                                        'seq' => count($classseq));
    }
    if (isset($row['rankid'])) {
      $rankseq[] = $row['rankid'];
      $ranks[$row['rankid']] = array('class' => $row['class'],
                                     'classid' => $row['classid'],
                                     'rank' => $row['rank'],
                                     'rankid' => $row['rankid'],
                                     'seq' => count($rankseq));
    }
  }
  return array($classes, $classseq, $ranks, $rankseq);
}

function update_ntrophies($classid, $ntrophies) {
  global $db;
  if (schema_version() >= 3) {
    $stmt = $db->prepare('UPDATE Classes SET ntrophies = :ntrophies'
                         .' WHERE classid = :classid');
    $stmt->execute(array(':classid' => $classid,
                         ':ntrophies' => $ntrophies));
  }
}
?>