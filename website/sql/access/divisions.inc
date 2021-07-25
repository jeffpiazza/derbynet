<?php

return array(

"DROP TABLE IF EXISTS Divisions",
"CREATE TABLE Divisions ("
."  divisionid        COUNTER,"
."  name              VARCHAR(200),"
."  sortorder         INTEGER"
.")",

"CREATE UNIQUE INDEX PrimaryKey ON Division(divisionid)",

"DROP TABLE IF EXISTS Subdivisions",
"CREATE TABLE Subdivisions ("
."  subdivisionid     COUNTER,"
."  name              VARCHAR(200),"
."  superdiv          INTEGER,"
."  sortorder         INTEGER"
.")",

"CREATE UNIQUE INDEX PrimaryKey ON Subivision(subdivisionid)"
);

?>