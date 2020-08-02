<?php

return array(

"DROP TABLE IF EXISTS RacingQueue",
"CREATE TABLE RacingQueue ("
."  queueid              INTEGER PRIMARY KEY,"
."  seq                  INTEGER,"
."  roundid              INTEGER,"
."  n_times_per_lane     INTEGER,"
."  sceneid_at_finish    INTEGER,"
."  continue_racing      INTEGER"
."  )",

);
?>