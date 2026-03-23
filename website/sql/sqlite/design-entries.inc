<?php
return array(
"DROP TABLE IF EXISTS `DesignEntries`",

"CREATE TABLE `DesignEntries` ("
." entryid INTEGER PRIMARY KEY, "
." racerid INTEGER NOT NULL, "
." awardid INTEGER NOT NULL"
.")",

"CREATE UNIQUE INDEX IF NOT EXISTS DesignEntries_RacerAward ON DesignEntries(racerid, awardid)",
make_index("DesignEntries", "racerid"),
make_index("DesignEntries", "awardid")
);
?>
