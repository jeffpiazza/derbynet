<?php
// $_POST['script']
start_response();

if (have_permission(SET_UP_PERMISSION)) {
  $ok = true;
  try {
    $sql_script = @include_once(sql_file_path($_POST['script']));
    if ($sql_script !== FALSE) {
      echo "<included-file-ok/>\n";
      foreach ($sql_script as $sql_stmt) {
        $sql_result = $db->exec($sql_stmt);
        echo '<executed result="'.($sql_result === false ? 'FALSE' : $sql_result).'">'
        .htmlspecialchars($sql_stmt, ENT_QUOTES, 'UTF-8')."</executed>\n";
        if ($sql_result === false) {
          echo '<failure code="sqlfailure">SQL statment execution failed.</failure>';
          $ok = false;
          break;
        }
      }
    } else {
      echo '<failure code="sqlnoscript">SQL script '.@$_POST['script'].' not found.</failure>'."\n";
      $ok = false;
    }
  } catch (PDOException $p) {
    echo '<failure code="sqlfailure">'.$p.'</failure>';
    $ok = false;
  }

  if ($ok) {
    echo "<success/>\n";
  }
} else {
  not_authorized_body();
}

end_response();
?>