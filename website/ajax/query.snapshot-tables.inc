<snapshot><![CDATA[
<?php
require_once('inc/export-schema.inc');

echo "[\n";
$first_table = true;
foreach ($schema as $table => $columns) {
  if ($first_table) {
    $first_table = false;
  } else {
    echo ",\n";
  }
  echo ' {"table": "'.$table.'",'."\n";
  echo '  "high-water": '.(0 + read_single_value('SELECT MAX('.$columns[0].') FROM '.$table)).",\n";
  echo '  "rows": ['."\n";
  $expecting = 1;
  $missed = array();
  $first_row = true;
  foreach ($db->query('SELECT '.implode(', ', $columns).' FROM '.$table.' ORDER BY '.$columns[0]) as $row) {
    if ($first_row) {
      $first_row = false;
    } else {
      echo ",\n";
    }
    while ($expecting < $row[$columns[0]]) {
      $missed[] = $expecting;
      ++$expecting;
    }
    echo "    {\n";
    $first_column = true;
    foreach ($columns as $column) {
      if ($first_column) {
        $first_column = false;
      } else {
        echo ",\n";
      }
      echo '     "'.$column.'": '.json_encode($row[$column]);
    }
    echo "}";
    ++$expecting;
  }
  echo "],\n";
  echo '  "missing": '.json_encode($missed)."\n";
  echo "}";
}
echo "]\n";

?>
]]></snapshot>