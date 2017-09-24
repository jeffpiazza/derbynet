<classes_and_ranks>
<?php
require_once('inc/schema_version.inc');

$stmt = $db->prepare('SELECT Classes.classid, class, rankid, rank, '
                     .' (SELECT COUNT(*) FROM RegistrationInfo WHERE RegistrationInfo.classid = Classes.classid) AS nclass,'
                     .' (SELECT COUNT(*) FROM RegistrationInfo WHERE RegistrationInfo.rankid = Ranks.rankid) AS nrank,'
                     .' (SELECT COUNT(*) FROM Rounds'
                     .'     WHERE Rounds.classid = Classes.classid'
                     .'       AND (SELECT COUNT(*) FROM RaceChart'
                     .'             WHERE RaceChart.roundid = Rounds.roundid'
                     .'               AND finishtime IS NOT NULL) > 0) AS nrounds'
                     .' FROM Classes'
                     .' LEFT JOIN Ranks ON Classes.classid = Ranks.classid'
                     .' ORDER BY '.(schema_version() >= 2
                                    ? 'Classes.sortorder, Ranks.sortorder, '
                                    : '')
                                  .'class, rank');

$stmt->execute(array());
$prev_class = '';
foreach ($stmt as $row) {
    if ($row['class'] != $prev_class) {
        if ($prev_class != '') {
            echo '</class>'."\n";
        }
        // count attribute is the number of racers registered in this class
        // nrounds attribute is the number of racing rounds *with results* for this class
        echo '<class classid="'.$row['classid'].'"'
            .' count="'.$row['nclass'].'"'
            .' nrounds="'.$row['nrounds'].'"'
            .' name="'.htmlspecialchars($row['class'], ENT_QUOTES, 'UTF-8').'">'."\n";
        $prev_class = $row['class'];
    }
    echo '  <rank rankid="'.$row['rankid'].'"'
        .' count="'.$row['nrank'].'"'
        .' name="'.htmlspecialchars($row['rank'], ENT_QUOTES, 'UTF-8').'"/>'."\n";
}

if ($prev_class != '') {
    echo '</class>'."\n";
}
?>
</classes_and_ranks>