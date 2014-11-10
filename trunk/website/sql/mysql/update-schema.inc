<?php
$sql_script = array();

if (schema_version() < 2) {
  $sql_script[] = "ALTER TABLE RaceInfo MODIFY COLUMN ItemValue VARCHAR(200)";

  $sql_script[] = "ALTER TABLE Classes ADD COLUMN sortorder INTEGER";
  $sql_script[] = "ALTER TABLE Ranks ADD COLUMN sortorder INTEGER";

  $sql_script[] = "ALTER TABLE RegistrationInfo ADD COLUMN carphoto VARCHAR(255)";
  
  $sql_script[] = "INSERT INTO RaceInfo(ItemKey, ItemValue) VALUES('schema', '2')"
      ." ON DUPLICATE KEY UPDATE ItemValue = '2'";
}
?>