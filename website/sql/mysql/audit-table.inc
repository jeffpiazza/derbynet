<?php
return array(

"DROP TABLE IF EXISTS CheckinAudit",

"CREATE TABLE CheckinAudit"
."	("
."	seq    INTEGER NOT NULL AUTO_INCREMENT,"
."	stmt   VARCHAR (250),"
."	tstamp DATETIME,"
."  PRIMARY KEY (seq)"
."	)",

);
?>