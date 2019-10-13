<?php

require_once('inc/schema_version.inc');


// Returns an array whose keys will include:
//
// connection_string
// scheme        one of: 'sqlite', 'odbc', or 'unknown'
//
// ... and, depending on scheme, might also include:
//
// sqlite_path
// odbc_dsn_name
function parse_connection_string($conn_string) {
  // If there's an existing database config, then $conn_string may
  // contain the connection string, which we can parse to better populate the form
  // fields.
  $result = array();

  if (isset($conn_string) && $conn_string) {
    $result['connection_string'] = $conn_string;
    if (substr($conn_string, 0, 7) == 'sqlite:') {
      $result['scheme'] = 'sqlite';
      $result['sqlite_path'] = substr($conn_string, 7);
    } else if (substr($conn_string, 0, 5) == 'odbc:') {
      if (preg_match('/DSN=([^;]*);/', $conn_string, $matches)) {
        $result['scheme'] = 'odbc';
        $result['odbc_dsn_name'] = $matches[1];
      } else {
        $result['scheme'] = 'unknown';
      }
    } else {
      $result['scheme'] = 'unknown';
    }
  } else {
    $result['scheme'] = 'empty';
    $result['connection_string'] = '';
  }

  return $result;
}

?>