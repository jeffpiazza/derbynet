<?php
require_once('inc/schema_version.inc');

if (!function_exists('make_index')) {
  function make_index($table, $column) {
    return "CREATE INDEX IF NOT EXISTS ".$table."_".$column." ON ".$table."(".$column.")";
  }

  function table_exists($table_name) {
    return read_single_value("SELECT COUNT(*) FROM sqlite_master"
                             ." WHERE type='table' AND name='$table_name'") > 0;
  }
}

$updates = array();

if (schema_version() < 2) {

  // There's no ALTER COLUMN in sqlite
  $updates[] = "CREATE TABLE TempRaceInfo AS SELECT * FROM RaceInfo";
  $updates[] = "DROP TABLE RaceInfo";
  $updates[] = "CREATE TABLE `RaceInfo` ("
      ."  `raceinfoid` INTEGER PRIMARY KEY, "
      ."  `itemkey` VARCHAR(20) NOT NULL, "
      ."  `itemvalue` VARCHAR(200)"
      .")";
  $updates[] = make_index("RaceInfo", "itemkey");
  $updates[] = "INSERT INTO RaceInfo SELECT * FROM TempRaceInfo";
  $updates[] = "DROP TABLE TempRaceInfo";

  $updates[] = "ALTER TABLE Classes ADD COLUMN sortorder INTEGER";
  $updates[] = "ALTER TABLE Ranks ADD COLUMN sortorder INTEGER";

  $updates[] = "ALTER TABLE RegistrationInfo ADD COLUMN carphoto VARCHAR(255)";

  $updates[] = "DELETE FROM RaceInfo WHERE itemkey = 'schema'";
  $updates[] = "INSERT INTO RaceInfo(itemkey, itemvalue) VALUES('schema', '2')";
}

if (schema_version() < 3) {
  $updates[] = "ALTER TABLE Classes ADD COLUMN constituents VARCHAR(100) DEFAULT ''";
  $updates[] = "ALTER TABLE Classes ADD COLUMN durable INTEGER";
  $updates[] = "ALTER TABLE Classes ADD COLUMN ntrophies INTEGER DEFAULT -1";

  $updates[] = "DELETE FROM RaceInfo WHERE itemkey = 'schema'";
  $updates[] = "INSERT INTO RaceInfo(itemkey, itemvalue) VALUES('schema', '3')";
}

if (schema_version() < 4) {
  if (!table_exists('MessageQueue')) {
    $updates = array_merge($updates,
                           @include(sql_file_path('message-queue-table')));
  }
  if (!table_exists('Scenes')) {
    $updates = array_merge($updates,
                           @include(sql_file_path('scene-tables')));
  }
  if (!table_exists('Playlist')) {
    $updates = array_merge($updates,
                           @include(sql_file_path('playlist-table')));
  }

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
  $updates = array_merge($updates,
                         @include(sql_file_path('partitions')));
  $updates[] = "ALTER TABLE RegistrationInfo ADD COLUMN partitionid INTEGER";
  $updates[] = "ALTER TABLE RegistrationInfo ADD COLUMN note VARCHAR(255)";
  $updates[] = "CREATE TABLE ConstituentClasses ("
                  ."antecedent INTEGER, dependent INTEGER, UNIQUE (antecedent, dependent))";

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

if (schema_version() < 10) {
  $updates = array_merge($updates,
                         @include(sql_file_path('action-history')));
  $updates[] = "ALTER TABLE Ranks ADD COLUMN ntrophies INTEGER DEFAULT -1";
  $updates[] = "ALTER TABLE RegistrationInfo ADD COLUMN checkin_time INTEGER";

  $updates[] = "DELETE FROM RaceInfo WHERE itemkey = 'schema'";
  $updates[] = "INSERT INTO RaceInfo(itemkey, itemvalue) VALUES('schema', '10')";
}

// For each new schema version, copy the DELETE and INSERT for schema version

return $updates;
?>
