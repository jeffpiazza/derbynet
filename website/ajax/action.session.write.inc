<?php

start_response();

foreach ($_POST as $key => $val) {
  if (substr($key, 0, strlen("session_")) == "session_") {
    $_SESSION[substr($key, strlen("session_"))] = $val;
  }
}

echo "<SESSION>\n";
echo htmlspecialchars(json_encode($_SESSION, JSON_PRETTY_PRINT), ENT_QUOTES, 'UTF-8');
echo "\n</SESSION>\n";

echo "<success/>\n";

end_response();

?>