<?php
return array(

"DROP TABLE IF EXISTS MessageQueue",
"CREATE TABLE MessageQueue ("
."  msgid      INTEGER NOT NULL AUTO_INCREMENT,"
."  recipient  VARCHAR(50),"
."  expiration INTEGER,"  // Unix timestamp
."  message    TEXT,"
.""
."  PRIMARY KEY (msgid)"
."  )",

);
?>
