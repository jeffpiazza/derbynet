<?php
require_once('inc/schema_version.inc');

$updates = array();

if (schema_version() < 2) {
  $updates[] = "ALTER TABLE RaceInfo ALTER COLUMN ItemValue VARCHAR(200)";

  $updates[] = "ALTER TABLE Classes ADD COLUMN SortOrder INTEGER";
  $updates[] = "ALTER TABLE Ranks ADD COLUMN SortOrder INTEGER";

  $updates[] = "ALTER TABLE RegistrationInfo ADD COLUMN CarPhoto VARCHAR(255)";
  
  $updates[] = "DELETE FROM RaceInfo WHERE ItemKey = 'schema'";
  $updates[] = "INSERT INTO RaceInfo(ItemKey, ItemValue) VALUES('schema', '2')";
}

if (schema_version() < 3) {
  $updates[] = "ALTER TABLE Classes ADD COLUMN constituents VARCHAR(100) DEFAULT ''";
  $updates[] = "ALTER TABLE Classes ADD COLUMN durable INTEGER";
  $updates[] = "ALTER TABLE Classes ADD COLUMN ntrophies INTEGER DEFAULT -1";

  $updates[] = "DELETE FROM RaceInfo WHERE itemkey = 'schema'";
  $updates[] = "INSERT INTO RaceInfo(itemkey, itemvalue) VALUES('schema', '3')";
}

if (schema_version() < 4) {
  $updates = array_merge($updates,
                         @include(sql_file_path('message-queue-table')),
                         @include(sql_file_path('scene-tables')),
                         @include(sql_file_path('playlist-table')));

  $updates[] = "DELETE FROM RaceInfo WHERE itemkey = 'schema'";
  $updates[] = "INSERT INTO RaceInfo(itemkey, itemvalue) VALUES('schema', '4')";
}

if (schema_version() < 5) {
  $updates[] = "ALTER TABLE Classes ADD COLUMN rankids VARCHAR(100) DEFAULT ''";

  $updates[] = "DELETE FROM RaceInfo WHERE itemkey = 'schema'";
  $updates[] = "INSERT INTO RaceInfo(itemkey, itemvalue) VALUES('schema', '5')";
}

if (schema_version() < 6) {
  $updates = array_merge($updates,
                         @include(sql_file_path('balloting')));

  $updates[] = "DELETE FROM RaceInfo WHERE itemkey = 'schema'";
  $updates[] = "INSERT INTO RaceInfo(itemkey, itemvalue) VALUES('schema', '6')";
}

if (schema_version() < 7) {
  $updates = array_merge($updates,
                         @include(sql_file_path('timer-settings')));

  $updates[] = "DELETE FROM RaceInfo WHERE itemkey = 'schema'";
  $updates[] = "INSERT INTO RaceInfo(itemkey, itemvalue) VALUES('schema', '7')";
}

if (schema_version() < 8) {
  $updates - array_merge($updates,
                         @include(sql_file_path('partitions')));
  $updates[] = "ALTER TABLE RegistrationInfo ADD COLUMN partitionid INTEGER";
  $updates[] = "ALTER TABLE RegistrationInfo ADD COLUMN note VARCHAR(255)";
  $updates[] = "CREATE TABLE ConstituentClasses ("
          ."  antecedent INTEGER, dependent  INTEGER)";
  $updates[] = "CREATE UNIQUE INDEX AnteDependent ON ConstituentClasses(antecedent, dependent)";
  
  $updates[] = "DELETE FROM RaceInfo WHERE itemkey = 'schema'";
  $updates[] = "INSERT INTO RaceInfo(itemkey, itemvalue) VALUES('schema', '8')";
}

if (schema_version() < 9) {
  $updates = array_merge($updates,
                         @include(sql_file_path('event-lane-details')));
  // $updates[] = "ALTER TABLE Events DROP COLUMN lane";
  $updates[] = "ALTER TABLE Events ADD COLUMN finishid INTEGER";

  $updates[] = "DELETE FROM RaceInfo WHERE itemkey = 'schema'";
  $updates[] = "INSERT INTO RaceInfo(itemkey, itemvalue) VALUES('schema', '9')";
}

// For each new schema version, copy the DELETE and INSERT for schema version

return $updates;
?>
