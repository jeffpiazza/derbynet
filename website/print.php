<?php
session_start();

// Usage e.g. /derbynet/print.php/racer/car-tags

if (isset($_SERVER['PATH_INFO'])) {
  $path_info = $_SERVER['PATH_INFO'];
} else if (isset($_SERVER['ORIG_PATH_INFO'])) {
  // TODO Obsolete?
  // Rewrite rules in Apache 2.2 may leave ORIG_PATH_INFO instead of PATH_INFO
  $path_info = 'print.php'.$_SERVER['ORIG_PATH_INFO'];
} else {
  echo "Debugging \$_SERVER:\n";
  var_export($_SERVER);
  exit(0);
}

$exploded = explode('/', $path_info);
while ($exploded[0] == '') {
  array_shift($exploded);
}
$inc = array_shift($exploded);

function document_class() {
  global $exploded;
  return $exploded[0];
}

function new_document() {
  $doc_class = document_class();
  return new $doc_class;
}

require_once('print/'.$inc.'.inc');
?>
