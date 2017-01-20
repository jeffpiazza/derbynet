<?php

if (!isset($_GET['path'])) {
  echo '<failure code="nopath">No PATH query argument provided</failure>'."\n";
} else {
  $path = $_GET['path'];
  echo '<path'
      .' readable="'.intval(is_readable($path)).'"'
      .' writable="'.intval(is_writable($path)).'"'
      .' directory="'.intval(is_dir($path)).'"'
      .'>';
  echo htmlspecialchars($path, ENT_QUOTES, 'UTF-8');
  echo '</path>'."\n";
}
?>