<?php

return array(

"DROP TABLE IF EXISTS Divisions",
"CREATE TABLE Divisions ("
."  divisionid        INTEGER PRIMARY KEY,"
."  name              VARCHAR(200) UNIQUE,"
."  rankid            INTEGER,"
."  sortorder         INTEGER"
.")",

);

?>