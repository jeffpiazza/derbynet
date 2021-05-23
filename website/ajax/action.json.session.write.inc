<?php

foreach ($_POST as $key => $val) {
  if (substr($key, 0, strlen("session_")) == "session_") {
    $_SESSION[substr($key, strlen("session_"))] = $val;
  }
}

json_out('session', $_SESSION);

json_success();

?>