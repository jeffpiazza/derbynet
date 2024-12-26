<snapshot><![CDATA[
<?php
require_once('inc/export-schema.inc');
require_once('inc/photo-config.inc');

// $_GET['clean'], if present, reduces first and last names to just an initial.

$do_clean = isset($_GET['clean']);
$export_photos = isset($_GET['photos']);

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
    if ($do_clean) {
      if (isset($row['firstname'])) {
        $row['firstname'] = substr($row['firstname'], 0, 1);
      }
      if (isset($row['lastname'])) {
        $row['lastname'] = substr($row['lastname'], 0, 1)."-".$row['racerid'];
      }
    }
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

function export_one_photo(&$row, &$repo) {
  if (empty(@$row[$repo->column_name()])) {
    return;
  }
  $file_path = $row[$repo->column_name()];
  $basename = basename($file_path);
  echo ",\n  \"".$repo->column_name()."\": {";
  echo "\n    \"basename\": \"$basename\"";

  if ($file_path == $repo->lookup(RENDER_CROPPED)->file_path($basename)) {
    $photo = @file_get_contents($repo->lookup(RENDER_ORIGINAL)->file_path($basename));
    if ($photo) {
      echo ",\n    \"original\": \"".bin2hex($photo)."\"";
    }
  }

  $photo = @file_get_contents($file_path);
  if ($photo) {
    echo ",\n    \"displayed\": \"".bin2hex($photo)."\"}";
  }
}

if ($export_photos) {
  foreach ($db->query('SELECT racerid, imagefile, carphoto FROM RegistrationInfo'
                      .' WHERE (imagefile IS NOT NULL AND imagefile <> \'\')'
                      .'    OR (carphoto IS NOT NULL AND carphoto <> \'\')'
  ) as $row) {
    echo ",\n";
    echo " {\"racerid\": $row[racerid]";
    export_one_photo($row, headshots());
    export_one_photo($row, car_photo_repository());
    echo "}";
  }
}

echo "]\n";

?>
]]></snapshot>
