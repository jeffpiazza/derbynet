<?php
return array(

"DROP TABLE IF EXISTS `Events`",
"CREATE TABLE `Events` ("
."  `seq`    INTEGER NOT NULL AUTO_INCREMENT,"
."	`tstamp` TIMESTAMP,"
."	`action` INTEGER,"
."  `racerid` INTEGER,"
."  `other`  VARCHAR(250),"
."  PRIMARY KEY (`seq`)"
."	)",
);

?>