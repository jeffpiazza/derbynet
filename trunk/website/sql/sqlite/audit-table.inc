<?php
return array(

"DROP TABLE IF EXISTS CheckinAudit",

"CREATE TABLE CheckinAudit"
."	("
."	seq    INTEGER PRIMARY KEY,"
."	stmt   VARCHAR (250),"
."	tstamp DATETIME"
."	)",

);
?>