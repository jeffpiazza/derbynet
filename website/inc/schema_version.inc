<?php

function schema_version() {
  return read_raceinfo('schema', 1);
}

function expected_schema_version() {
  return 10;
}

define('RANK_AGGREGATE_SCHEMA', 5);  // Classes.rankids for aggregates formed from ranks
define('BALLOTING_SCHEMA', 6);  // First schema version to support balloting
define('TIMER_SETTINGS_SCHEMA', 7);
define('PARTITION_SCHEMA', 8);
define('TIMER_RESULT_SCHEMA', 9);
define('PER_SUBGROUP_AWARDS_SCHEMA', 10);
define('REGISTRATION_CHECKIN_SCHEMA', 10);
?>
