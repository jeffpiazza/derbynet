<?php
return array(

'DROP TABLE CheckinAudit',

'CREATE TABLE CheckinAudit'
.'	('
.'	seq    COUNTER,'
.'	stmt   VARCHAR (250),'
.'	tstamp DATETIME'
.'	)',

);
?>