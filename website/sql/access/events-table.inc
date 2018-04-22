<?php

return array(

"DROP TABLE Events",
"CREATE TABLE Events ("
."  seq    COUNTER,"
."	tstamp DATETIME,"
."	action INTEGER,"
."  racerid INTEGER,"
."  other  VARCHAR(250),"
."	)",

"CREATE UNIQUE INDEX PrimaryKey"
."  ON Events (seq)"

);

?>