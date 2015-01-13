<?php
require_once('inc/schema_version.inc');

$updates = array();

if (schema_version() < 2) {
  $updates[] = "ALTER TABLE RaceInfo MODIFY COLUMN ItemValue VARCHAR(200)";

  $updates[] = "ALTER TABLE Classes ADD COLUMN SortOrder INTEGER";
  $updates[] = "ALTER TABLE Ranks ADD COLUMN SortOrder INTEGER";

  $updates[] = "ALTER TABLE RegistrationInfo ADD COLUMN CarPhoto VARCHAR(255)";
  
  $updates[] = "INSERT INTO RaceInfo(ItemKey, ItemValue) VALUES('schema', '2')"
      ." ON DUPLICATE KEY UPDATE ItemValue = '2'";
}

return $updates;
?>