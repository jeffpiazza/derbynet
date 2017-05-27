<?php

function sql_file_path($stem) {
  global $dbtype;
  return 'sql'.DIRECTORY_SEPARATOR.$dbtype.DIRECTORY_SEPARATOR.$stem.'.inc';
}

function report_sql_result($sql_result, $sql_stmt) {
  echo '<executed result="'.($sql_result === false ? 'FALSE' : $sql_result).'">'
      .htmlspecialchars($sql_stmt, ENT_QUOTES, 'UTF-8')."</executed>\n";
}

function run_sql_script($scriptname, $write_xml = false) {
  global $db;
  global $dbtype;
  $ok = true;
  try {
    $sql_script = @include_once(sql_file_path($scriptname));
    if ($sql_script !== FALSE) {
      foreach ($sql_script as $sql_stmt) {
        try {
          $sql_result = $db->exec($sql_stmt);
          if ($write_xml) {
            report_sql_result($sql_result, $sql_stmt);
          }
        } catch (PDOException $p) {
          report_sql_result($sql_result, $sql_stmt);

          // Hack to work around the fact that Access doesn't provide a "drop if
          // exists" statement -- you have to attempt a table drop and catch the
          // failure if the table didn't already exist.
          if ($dbtype == 'access' && substr($sql_stmt, 0, 4) == 'DROP') {
            if ($write_xml) {
              echo '<drop-failed/>';
            }
          } else {
            if ($write_xml) {
              echo '<failure code="sqlfailure">'.htmlspecialchars($p->getMessage(), ENT_QUOTES, 'UTF-8')."</failure>\n";
              foreach ($p->errorInfo as $info) {
                echo "<failure-info>".htmlspecialchars($info, ENT_QUOTES, 'UTF-8')."</failure-info>\n";
              }
            }
            $ok = false;
            break;
          }
        }
      }
    } else {
      if ($write_xml) {
        echo '<failure code="sqlnoscript">SQL script '.@$_POST['script'].' not found.</failure>'."\n";
      }
      $ok = false;
    }
  } catch (PDOException $p) {
    if ($write_xml) {
      echo '<failure code="sqlfailure">'.$p.'</failure>';
    }
    $ok = false;
  }

  if ($write_xml && $ok) {
    echo "<success/>\n";
  }

  return $ok;
}

?>