<?php

return array(

"DROP TABLE IF EXISTS Divisions",
"CREATE TABLE Divisions ("
."  divisionid        INTEGER PRIMARY KEY,"
."  name              VARCHAR(200) UNIQUE,"
."  sortorder         INTEGER"
.")",

"DROP TABLE IF EXISTS Subdivisions",
"CREATE TABLE Subdivisions ("
."  subdivisionid     INTEGER PRIMARY KEY,"
."  name              VARCHAR(200),"
."  superdiv          INTEGER,"
."  sortorder         INTEGER,"
."  UNIQUE (name, superdiv)"
.")"

);

?>