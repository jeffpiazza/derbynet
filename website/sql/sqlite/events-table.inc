<?php
return array(

"DROP TABLE IF EXISTS Events",
"CREATE TABLE Events ("
."  seq    INTEGER PRIMARY KEY,"
."	tstamp DATETIME,"
."	action INTEGER,"
."  racerid INTEGER,"
."  other  VARCHAR(250)"
."	)",
);
?>