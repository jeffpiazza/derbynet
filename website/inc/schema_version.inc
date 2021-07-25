<?php

function schema_version() {
  return read_raceinfo('schema', 1);
}

function expected_schema_version() {
  return 8;
}

define('BALLOTING_SCHEMA', 6);  // First schema version to support balloting
define('TIMER_SETTINGS_SCHEMA', 7);
define('DIVISIONS_SCHEMA', 8);
?>