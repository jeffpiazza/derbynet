<?php

require_once('inc/data.inc');
require_once('inc/f-inv.inc');

function lane_bias_analysis($emit_rows = false) {
  global $db;

  $time_format = get_finishtime_formatting_string();

  $stmt = $db->prepare('SELECT finishtime FROM RaceChart'
                       .' WHERE completed IS NOT NULL AND completed <> \'\''
                       .'  AND finishtime IS NOT NULL AND finishtime <> 0.0'
                       .'  AND lane = :lane');
  $nlanes = get_lane_count();
  $lanes = array();
  $total = 0.0;
  $nn = 0;
  $dnfs = 0;
  for ($l = 1; $l <= $nlanes; ++$l) {
    $lanes[] = array('n' => 0, 'dnfs' => 0, 'total' => 0.0, 'mean' => 0.0, 'sswithin' => 0.0);
    $lane = &$lanes[$l - 1];
    $stmt->execute(array(':lane' => $l));
    foreach ($stmt as $time) {
      if ($time['finishtime'] < 9.9) {
        $lane['n'] += 1;
        $lane['total'] += $time['finishtime'];
      } else {
        ++$lane['dnfs'];
      }
    }
    $lane['mean'] = $lane['total'] / $lane['n'];
    $stmt->execute(array(':lane' => $l));
    foreach ($stmt as $time) {
      if ($time['finishtime'] < 9.9) {
        $lane['sswithin'] += ($time['finishtime'] - $lane['mean']) * ($time['finishtime'] - $lane['mean']);
      }
    }
    $total += $lane['total'];
    $nn += $lane['n'];
    $dnfs += $lane['dnfs'];

    if ($emit_rows) {
      echo "<tr>";
      echo "<td>Lane ".$l."</td>";

      echo "<td>".($lane['n'] + $lane['dnfs'])." run(s)</td>";
      echo "<td>".$lane['dnfs']." DNF(s)</td>";
      echo "<td>".sprintf($time_format, $lane['mean'])."s avg.</td>";
      echo "<td>".sprintf($time_format, $lane['sswithin'] / ($lane['n'] - 1))." sample variance.</td>";
      echo "</tr>\n";
    }
  }

  if ($nn + $dnfs == 0) {
    return array('biased' => false);
  }

  $msbetween = $ssbetween / ($nlanes - 1);
  $mswithin = $sswithin / ($nn - $nlanes);
  $f_stat = $msbetween / $mswithin;
  $df1 = $nlanes - 1;
  $df2 = $nn - $nlanes;
  $f_critical = critical_f_value($df1, $df2);

  return array('biased' => $f_stat > $f_critical,
               'f-statistic' => $f_stat,
               'df1' => $df1,
               'df2' => $df2,
               'critical-value' => $f_critical);
}

?>