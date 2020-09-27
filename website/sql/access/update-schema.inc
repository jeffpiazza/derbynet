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
                           @include_once(sql_file_path('message-queue-table')),
                           @include_once(sql_file_path('scene-tables')),
                           @include_once(sql_file_path('playlist-table')));

  $updates[] = "DELETE FROM RaceInfo WHERE itemkey = 'schema'";
  $updates[] = "INSERT INTO RaceInfo(itemkey, itemvalue) VALUES('schema', '4')";
}

return $updates;
?>