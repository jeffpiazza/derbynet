<?php

"DROP TABLE IF EXISTS TimerResults",
// Raw results reported by the timer, along with expected round and heat
"CREATE TABLE TimerResults ("
."  tresultid           INTEGER PRIMARY KEY, "
."  `expected_roundid` INTEGER NOT NULL, "
."  `expected_heat` INTEGER NOT NULL, "
."  `lane` INTEGER NOT NULL, "
."  `finishtime` DOUBLE NULL, "
."  `finishplace` INTEGER, "
."  `completed`   DATETIME"
.")",

);

?>