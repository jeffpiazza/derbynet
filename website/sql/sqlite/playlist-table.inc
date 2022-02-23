<?php

return array(

"DROP TABLE IF EXISTS Playlist",
"CREATE TABLE Playlist ("
."  queueid              INTEGER PRIMARY KEY,"
."  seq                  INTEGER,"
// Identifying the round by classid + round.
."  classid              INTEGER,"
."  round                INTEGER,"
// Roster creation parameters (if needed):
."  bucket_limit         INTEGER,"
."  bucketed             INTEGER,"  // Boolean
// Scheduling parameter (if needed): how many runs per lane for each racer
."  n_times_per_lane     INTEGER,"
// If sceneid_at_finish is non-zero, it's a foreign key to Scenes table.
."  sceneid_at_finish    INTEGER,"
// If sceneid_at_finish is zero, then continue_racing says whether to advance
// directly to the next round without a break.
."  continue_racing      INTEGER"
."  )",

);
?>