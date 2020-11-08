<?php

function schema_version() {
  return read_raceinfo('schema', 1);
}

function expected_schema_version() {
  return 6;
}

define('BALLOTING_SCHEMA', 6);  // First schema version to support balloting
?>