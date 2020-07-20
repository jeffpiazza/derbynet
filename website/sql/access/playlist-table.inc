<?php

return array(

"DROP TABLE IF EXISTS Playlist",
"CREATE TABLE Playlist ("
."  playlistid           COUNTER,"
."  seq                  INTEGER,"
."  roundid              INTEGER,"
."  n_times_per_lane     INTEGER,"
."  sceneid_at_finish    INTEGER,"
."  continue_racing      INTEGER"
."  )",

"CREATE UNIQUE INDEX PrimaryKey ON Playlist(playlistid)"

);
?>