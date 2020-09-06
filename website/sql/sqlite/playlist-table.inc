<?php

return array(

"DROP TABLE IF EXISTS Playlist",
"CREATE TABLE Playlist ("
."  queueid              INTEGER PRIMARY KEY,"
."  seq                  INTEGER,"
// Identifying the round, either by classid + round or by roundid.
."  classid              INTEGER,"
."  round                INTEGER,"
// Roster creation parameters (if needed):
."  bucket_limit         INTEGER,"
."  bucketed             INTEGER,"  // Boolean
// Scheduling parameter (if needed): how many runs per lane for each racer
."  n_times_per_lane     INTEGER,"
."  sceneid_at_finish    INTEGER,"
."  continue_racing      INTEGER"
."  )",

);
?>