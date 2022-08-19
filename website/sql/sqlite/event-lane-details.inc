<?php

return array(

"DROP TABLE IF EXISTS EventLaneDetails",

// Raw results reported by the timer, or entered by the user,
// along with the roundid and heat that were current at the time
// (i.e., the heat to which these values were applied).
"CREATE TABLE EventLaneDetails ("
."  tresultid           INTEGER PRIMARY KEY, "
."  finishid            INTEGER, "
."  `lane`              INTEGER NOT NULL, "
."  `finishtime`        DOUBLE NULL, "
."  `finishplace`       INTEGER, "
."  `completed`         TIMESTAMP DEFAULT CURRENT_TIMESTAMP "
.")",

"CREATE INDEX IF NOT EXISTS EventLaneDetails_finishid ON EventLaneDetails(finishid)",

);

?>
