<?php
return array(

"DROP TABLE MessageQueue",
"CREATE TABLE MessageQueue ("
."  msgid      COUNTER,"
."  recipient  VARCHAR(50),"
."  expiration INTEGER,"  // Unix timestamp
."  message    TEXT"
."  )",

);
?>
