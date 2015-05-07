<classes_and_ranks>
<?php
require_once('inc/schema_version.inc');

$stmt = $db->prepare('SELECT Classes.classid, class, rankid, rank'
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
        echo '<class classid="'.$row['classid'].'" name="'.htmlspecialchars($row['class'], ENT_QUOTES, 'UTF-8').'">'."\n";
        $prev_class = $row['class'];
    }
    echo '  <rank rankid="'.$row['rankid'].'" name="'.htmlspecialchars($row['rank'], ENT_QUOTES, 'UTF-8').'"/>'."\n";
}

if ($prev_class != '') {
    echo '</class>'."\n";
}
?>
</classes_and_ranks>