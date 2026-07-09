<?php
// $_GET['awardid'] - The award to get entry count for

$awardid = $_GET['awardid'];

$count = read_single_value('SELECT COUNT(*) FROM DesignEntries WHERE awardid = :awardid',
                         array(':awardid' => $awardid));

json_out('entry_count', $count);
json_success();
?>
