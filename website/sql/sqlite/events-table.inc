<?php
return array(

"DROP TABLE IF EXISTS Events",
"CREATE TABLE Events ("
."  seq     INTEGER PRIMARY KEY,"
."	tstamp  DATETIME,"
."	action  INTEGER,"
."  racerid INTEGER,"
."  classid INTEGER,"
."  rankid  INTEGER,"
."  roundid INTEGER,"
."  heat    INTEGER,"
."  finishid  INTEGER,"  // FK to EventLaneDetails.finishid
."  other   VARCHAR(250)"
."	)",
);
?>
