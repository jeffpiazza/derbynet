<?php

return array(

"DROP TABLE Events",
"CREATE TABLE Events ("
."  seq     COUNTER,"
."	tstamp  DATETIME,"
."	action  INTEGER,"
."  racerid INTEGER,"
."  classid INTEGER,"
."  rankid  INTEGER,"
."  roundid INTEGER,"
."  heat    INTEGER,"
."  finishid INTEGER,"  // FK to EventLaneDetails.finishid
."  other   VARCHAR(250),"
."	)",

"CREATE UNIQUE INDEX PrimaryKey"
."  ON Events (seq)"

);

?>
