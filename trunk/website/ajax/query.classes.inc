<classes_and_ranks>
<?php
$stmt = $db->prepare('SELECT class, rank'
                     .' FROM Classes'
                     .' INNER JOIN Ranks ON Classes.classid = Ranks.classid'
                     .' ORDER BY class, rank');
$stmt->execute(array());
$prev_class = '';
foreach ($stmt as $row) {
    if ($row['class'] != $prev_class) {
        if ($prev_class != '') {
            echo '</class>'."\n";
        }
        echo '<class name="'.htmlspecialchars($row['class'], ENT_QUOTES).'">'."\n";
        $prev_class = $row['class'];
    }
    echo '  <rank name="'.htmlspecialchars($row['rank'], ENT_QUOTES).'"/>'."\n";
}

if ($prev_class != '') {
    echo '</class>'."\n";
}
?>
</classes_and_ranks>