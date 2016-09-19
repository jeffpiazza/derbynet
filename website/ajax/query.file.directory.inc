<?php

require_once('inc/action-helpers.inc');

if (!isset($_GET['path'])) {
   echo '<failure code="nopath">No PATH query argument provided</failure>'."\n";
} else {
  $dirpath = $_GET['path'];

  echo '<directory original="'.htmlspecialchars($dirpath, ENT_QUOTES, 'UTF-8').'">'."\n";

  while (is_link($dirpath)) {
    $dirpath = readlink($dirpath);
  }

  if (is_file($dirpath)) {
    $basename = pathinfo($dirpath, PATHINFO_BASENAME);
    $dirpath = pathinfo($dirpath, PATHINFO_DIRNAME);
    echo "<chosen>".htmlspecialchars($basename, ENT_QUOTES, 'UTF-8')."</chosen>\n";
  }

  while (true) {
    if ($dirpath === "") {
      $dirpath = DIRECTORY_SEPARATOR;
    }
    $files = @scandir($dirpath);
    if ($files !== false) {
      break;
    }
    echo "  <failure code='unable'>"
         ."Unable to list directory ".htmlspecialchars($dirpath, ENT_QUOTES, 'UTF-8')
         ."</failure>\n";
    $info = pathinfo($dirpath, PATHINFO_DIRNAME);
    if ($dirpath == $info) {
      // Nothing left
      break;
    } else {
      $dirpath = $info;
    }
  }

  if (substr($dirpath, -1) !== DIRECTORY_SEPARATOR) {
    $dirpath .= DIRECTORY_SEPARATOR;
  }

  $realpath = realpath($dirpath);
  if (substr($realpath, -1) !== DIRECTORY_SEPARATOR) {
    $realpath .= DIRECTORY_SEPARATOR;
  }
  echo '<path realpath="'.htmlspecialchars($realpath, ENT_QUOTES, 'UTF-8').'"/>'."\n";

  $superpath = $realpath;
  while ($superpath !== "") {
    $basename = pathinfo($superpath, PATHINFO_BASENAME);
    echo '  <base path="'.htmlspecialchars($superpath, ENT_QUOTES, 'UTF-8').'">'
        .htmlspecialchars($basename, ENT_QUOTES, 'UTF-8').'</base>'."\n";
    $info = pathinfo($superpath, PATHINFO_DIRNAME);
    if ($superpath == $info) {
      break;
    } else {
      $superpath = $info;
    }
  }

  if ($files === false) {
  } else {  
    foreach ($files as $file) {
      // if ($file != "." && $file != "..") {
      if (substr($file, 0, 1) != ".") {
        $path = $dirpath . $file;
        echo '  <file'
            .' readable="'.intval(is_readable($path)).'"'
            .' writable="'.intval(is_writable($path)).'"'
            .' directory="'.intval(is_dir($path)).'"'
            .'>';
        echo htmlspecialchars($file, ENT_QUOTES, 'UTF-8');
        echo '</file>'."\n";
      }
    }
  }

  echo "</directory>\n";
}
?>
