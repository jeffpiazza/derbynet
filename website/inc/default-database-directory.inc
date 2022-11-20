<?php
// Installer may leave a file to suggest where to put the database and/or photo directories.

function default_database_directory() {
  if (isset($_SERVER['DERBYNET_DATA_DIR'])) {
    return realpath($_SERVER['DERBYNET_DATA_DIR']);
  }
  return "";
}
?>
