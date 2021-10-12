<?php

return array(

"DROP TABLE IF EXISTS Partitions",
"CREATE TABLE Partitions ("
."  partitionid        INTEGER PRIMARY KEY,"
."  name              VARCHAR(200) UNIQUE,"
."  rankid            INTEGER,"
."  sortorder         INTEGER"
.")",

);

?>