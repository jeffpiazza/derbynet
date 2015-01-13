<?php
return array(

"DROP TABLE IF EXISTS Kiosks",
"CREATE TABLE Kiosks ("
." address VARCHAR(50),"
." name VARCHAR(200),"
." page VARCHAR(200),"
." last_contact INTEGER NOT NULL)",

);
?>