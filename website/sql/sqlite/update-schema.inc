<?php
require_once('inc/schema_version.inc');

function make_index($table, $column) {
  return "CREATE INDEX IF NOT EXISTS ".$table."_".$column." ON ".$table."(".$column.")";
}

$sql_script = array();

if (schema_version() < 2) {

  // There's no MODIFY COLUMN in sqlite
  $sql_script[] = "CREATE TABLE TempRaceInfo AS SELECT * FROM RaceInfo";
  $sql_script[] = "DROP TABLE RaceInfo";
  $sql_script[] = "CREATE TABLE `RaceInfo` ("
      ."  `raceinfoid` INTEGER PRIMARY KEY, "
      ."  `itemkey` VARCHAR(20) NOT NULL, "
      ."  `itemvalue` VARCHAR(200)"
      .")";
  $sql_script[] = make_index("RaceInfo", "itemkey");
  $sql_script[] = "INSERT INTO RaceInfo SELECT * FROM TempRaceInfo";
  $sql_script[] = "DROP TABLE TempRaceInfo";

  $sql_script[] = "ALTER TABLE Classes ADD COLUMN sortorder INTEGER";
  $sql_script[] = "ALTER TABLE Ranks ADD COLUMN sortorder INTEGER";

  $sql_script[] = "ALTER TABLE RegistrationInfo ADD COLUMN carphoto VARCHAR(255)";

  $sql_script[] = "INSERT OR REPLACE INTO RaceInfo(ItemKey, ItemValue) VALUES('schema', '2')";
}
?>