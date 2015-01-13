<?php
require_once('inc/schema_version.inc');

function make_index($table, $column) {
  return "CREATE INDEX IF NOT EXISTS ".$table."_".$column." ON ".$table."(".$column.")";
}

$updates = array();

if (schema_version() < 2) {

  // There's no MODIFY COLUMN in sqlite
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

  $updates[] = "INSERT OR REPLACE INTO RaceInfo(ItemKey, ItemValue) VALUES('schema', '2')";
}

return $updates;
?>