<?php
return array(

"DROP TABLE IF EXISTS MessageQueue",
"CREATE TABLE MessageQueue ("
."  msgid      INTEGER PRIMARY KEY, "
."  recipient  VARCHAR(50),"
."  expiration INTEGER,"  // Unix timestamp
."  message    TEXT"
."  )",

);
?>
