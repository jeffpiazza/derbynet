<?php

return array(

"DROP TABLE IF EXISTS Partitions",
"CREATE TABLE Partitions ("
."  partitionid        COUNTER,"
."  name              VARCHAR(200),"
."  rankid            INTEGER,"
."  sortorder         INTEGER"
.")",

"CREATE UNIQUE INDEX PrimaryKey ON Partition(partitionid)",
);

?>