<?php
require_once('inc/schema_version.inc');

$sql_script = array();

if (schema_version() < 2) {
  $sql_script[] = "ALTER TABLE RaceInfo ALTER COLUMN ItemValue VARCHAR(200)";

  $sql_script[] = "ALTER TABLE Classes ADD COLUMN SortOrder INTEGER";
  $sql_script[] = "ALTER TABLE Ranks ADD COLUMN SortOrder INTEGER";

  $sql_script[] = "ALTER TABLE RegistrationInfo ADD COLUMN CarPhoto VARCHAR(255)";
  
  $sql_script[] = "DELETE FROM RaceInfo WHERE ItemKey = 'schema'";
  $sql_script[] = "INSERT INTO RaceInfo(ItemKey, ItemValue) VALUES('schema', '2')";
}
?>