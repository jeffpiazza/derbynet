<?php

return array(

"DROP TABLE IF EXISTS ActionHistory",
"CREATE TABLE ActionHistory ("
." historyid    INTEGER PRIMARY KEY,"
." received     TIMESTAMP,"
." request      TEXT"
.")",

);

?>
