<?php

return array(

"DROP TABLE IF EXISTS Divisions",
"CREATE TABLE Divisions ("
."  divisionid        COUNTER,"
."  name              VARCHAR(200),"
."  rankid            INTEGER,"
."  sortorder         INTEGER"
.")",

"CREATE UNIQUE INDEX PrimaryKey ON Division(divisionid)",
);

?>