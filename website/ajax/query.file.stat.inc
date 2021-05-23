<?php

if (!isset($_GET['path'])) {
  json_failure('nopath', "No PATH query argument provided");
} else {
  $path = $_GET['path'];
  json_out('stat', array('file' => $path,
                         'readable' => is_readable($path),
                         'writable' => is_writable($path),
                         'isdir' => is_dir($path)));
}
?>