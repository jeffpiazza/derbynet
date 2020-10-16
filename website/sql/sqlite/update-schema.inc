<?php
require_once('inc/schema_version.inc');

function make_index($table, $column) {
  return "CREATE INDEX IF NOT EXISTS ".$table."_".$column." ON ".$table."(".$column.")";
}

function table_exists($table_name) {
  return read_single_value("SELECT COUNT(*) FROM sqlite_master"
                           ." WHERE type='table' AND name='$table_name'") > 0;
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
                           @include_once(sql_file_path('message-queue-table')));
  }
  if (!table_exists('Scenes')) {
    $updates = array_merge($updates,
                           @include_once(sql_file_path('scene-tables')));
  }
  if (!table_exists('Playlist')) {
    $updates = array_merge($updates,
                           @include_once(sql_file_path('playlist-table')));
  }

  $updates[] = "DELETE FROM RaceInfo WHERE itemkey = 'schema'";
  $updates[] = "INSERT INTO RaceInfo(itemkey, itemvalue) VALUES('schema', '4')";
}

if (schema_version() < 5) {
  $updates[] = "ALTER TABLE Classes ADD COLUMN rankids VARCHAR(100) DEFAULT ''";

  $updates[] = "DELETE FROM RaceInfo WHERE itemkey = 'schema'";
  $updates[] = "INSERT INTO RaceInfo(itemkey, itemvalue) VALUES('schema', '5')";
}

return $updates;
?>