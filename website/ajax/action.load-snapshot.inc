<?php

/* The uploaded file's content is json text wrapped in a thin veneer of XML.
   (The XML is because our ajax queries are assumed to return XML, and it was
   easier to comply than fight.)

   The json content is an array of json objects, one per database table:
   {table:  <table name>
    high-water: <highest id number>
    rows: <array of row objects>
    missing: <array of id values>
   }

   Each row object is an associative array (in PHP) of column name to column value.
*/

require_once('inc/export-schema.inc');

start_response();
echo "\n";
echo "<uploaded-file name=\"".htmlspecialchars($_FILES['snapshot']['name'], ENT_QUOTES, 'UTF-8')."\""
  ." size=\"".$_FILES['snapshot']['size']."\""
  .">\n";

$xml = file_get_contents($_FILES['snapshot']['tmp_name']);

$start_marker = '<snapshot><![CDATA[';
$stop_marker = ']]></snapshot>';

$pos = strpos($xml, $start_marker) + strlen($start_marker);
$rpos = strrpos($xml, $stop_marker);

$json = json_decode(substr($xml, $pos, $rpos - $pos), true);

if (!$json) {
  echo "FAILED:\n";
  echo $xml;
}

foreach ($json as $table_entry) {
  $tablename = $table_entry["table"];
  $schema_columns = $schema[$tablename];  // An array of column names, id column first
  $id_column_name = $schema_columns[0];
  $try_again = true;
  for ($attempts = 0; $try_again && $attempts < 10; ++$attempts) {
    $try_again = false;
    echo '<table name="'.$tablename.'"'
        .' rows-before="'.read_single_value('SELECT COUNT(*) FROM '.$tablename).'"'
        .' idcolumn="'.$id_column_name.'"'
        .'>'."\n";
    $row_count = 0;
    $difference_count = 0;
    $insertion_count = 0;
    $fetch_stmt = $db->prepare('SELECT * FROM '.$tablename.' WHERE '.$id_column_name.' = :id');
    foreach ($table_entry["rows"] as $json_row) {
      ++$row_count;
      $fetch_stmt->execute(array(':id' => $json_row[$id_column_name]));
      $row = $fetch_stmt->fetch(PDO::FETCH_ASSOC);
      $fetch_stmt->closeCursor();

      if ($row) {
        $need_update = false;
        foreach ($json_row as $column_name => $value) {
          // TODO nulls and other special values
          if ($row[$column_name] !== $value) {
            $need_update = true;
            ++$difference_count;
            break;
          }
        }
        if ($need_update) {
          $usql = 'UPDATE '.$tablename;
          $uparams = array();
          $first = true;
          foreach ($json_row as $c => $v) {
            $usql .= ($first ? ' SET ' : ', ').$c.' = :'.$c;
            $uparams[':'.$c] = $v;
            $first = false;
          }
          $usql .= ' WHERE '.$id_column_name.' = :id';
          $uparams[':id'] = $json_row[$id_column_name];
          $stmt = $db->prepare($usql);
          try {
            if (!$stmt->execute($uparams)) {
              echo "<failure code='update'>Can't update ".$tablename
                  ." for id=".$json_row[$id_column_name]."</failure>\n";
            }
          } catch (Exception $e) {
            echo "  <exception>".$e->getMessage()."</exception>\n";
            $try_again = true;
          }
        }
      } else {
        ++$insertion_count;
        $write_sql = 'INSERT INTO '.$tablename;
        $first = true;
        $write_values = '';
        $write_params = array();
        foreach ($json_row as $column_name => $value) {
          $write_sql .= ($first ? '(' : ', ').$column_name;
          $write_values .= ($first ? '' : ', ').':'.$column_name;
          $write_params[':'.$column_name] = $value;
          $first = false;
        }
        $write_sql .= ') VALUES ('.$write_values.')';
        $stmt = $db->prepare($write_sql);
        try {
          if (!$stmt->execute($write_params)) {
            echo "<failure code='insert'>Can't insert ".$tablename." id=".$json_row[$id_column_name]
            ."</failure>\n";
          }
        } catch (Exception $e) {
          echo "  <exception>".$e->getMessage()."</exception>\n";
          $try_again = true;
        }
      }
    }
    echo '  <table-summary rows-received="'.$row_count.'" rows-inserted="'.$insertion_count.'"'
        .' columns-updated="'.$difference_count.'"/>'."\n";
    echo "</table>\n";
  }
}

// For the deletions, go through tables in reverse order, to be sure to remove foreign keys
// before corresponding primary keys.
  foreach (array_reverse($json) as $table_entry) {
    $tablename = $table_entry["table"];
    $schema_columns = $schema[$tablename];  // An array of column names, id column first
    $id_column_name = $schema_columns[0];

    try {
      $db->exec('DELETE FROM '.$tablename
                .' WHERE '.$id_column_name.' > '.$table_entry["high-water"]
                .(count($table_entry["missing"]) > 0
                  ? '  OR '.$id_column_name.' IN (0,'.implode(',', $table_entry["missing"]).')'
                  : ''));
    } catch (Exception $e) {
      echo "  <exception>".$e->getMessage()."</exception>\n";
      $try_again = true;
    }
  }

if ($try_again) {
  echo "<failure code='too_many_tries'>Still unable to sync after retries</failure>\n";
} else {
  echo "<success/>\n";
}

echo "</uploaded-file>\n";
end_response();
?>