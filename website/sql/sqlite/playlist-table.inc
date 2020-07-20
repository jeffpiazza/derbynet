<?php

return array(

"DROP TABLE IF EXISTS Playlist",
"CREATE TABLE Playlist ("
."  playlistid           INTEGER PRIMARY KEY,"
."  seq                  INTEGER,"
."  roundid              INTEGER,"
."  n_times_per_lane     INTEGER,"
."  sceneid_at_finish    INTEGER,"
."  continue_racing      INTEGER"
."  )",

);
?>