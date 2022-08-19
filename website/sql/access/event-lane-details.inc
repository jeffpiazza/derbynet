<?php

return array(
  
"DROP TABLE IF EXISTS EventLaneDetails",

// Raw results reported by the timer, along with expected round and heat
"CREATE TABLE EventLaneDetails ("
."  tresultid           INTEGER PRIMARY KEY, "
."  finishid            INTEGER, "
."  `lane`              INTEGER NOT NULL, "
."  `finishtime`        DOUBLE NULL, "
."  `finishplace`       INTEGER, "
."  `completed`         DATETIME"
.")",

"CREATE INDEX EventLaneDetails_finishid ON EventLaneDetails(finishid)",

);

?>
