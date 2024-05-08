<?php
session_start();
require_once('inc/data.inc');
session_write_close();

// Usage e.g. /derbynet/print.php/racer/CarTagsDocument

//  ["DOCUMENT_URI"]=> "/derbynet/pack153/2019/render-document.php/racer/CarTagsDocument"
//  ["SCRIPT_NAME"]=> "/derbynet/pack153/2019/render-document.php"

// We're trying to extract the "racer/CarTagsDocument" part into $args variable

function confirm_args($str) {
  $ex = explode('/', $str);
  while (count($ex) > 0 && $ex[0] == '') {
    array_shift($ex);
  }
  if (count($ex) == 0) {
    return false;
  }
  return file_exists(__DIR__.'/print/render/'.$ex[0].'.inc');
}

$have_args = false;

if (!$have_args && isset($_SERVER['DOCUMENT_URI']) && isset($_SERVER['SCRIPT_NAME']) &&
    substr($_SERVER['DOCUMENT_URI'], 0, strlen($_SERVER['SCRIPT_NAME'])) == $_SERVER['SCRIPT_NAME']) {
  $args = substr($_SERVER['DOCUMENT_URI'], strlen($_SERVER['SCRIPT_NAME']));
  $have_args = confirm_args($args);
}

if (!$have_args && isset($_SERVER['PATH_INFO'])) {
  $args = $_SERVER['PATH_INFO'];
  $have_args = confirm_args($args);
}

if (!$have_args && isset($_SERVER['ORIG_PATH_INFO'])) {
  // Rewrite rules e.g. for hosted DerbyNet may leave ORIG_PATH_INFO instead of PATH_INFO
  $args = $_SERVER['ORIG_PATH_INFO'];
  $have_args = confirm_args($args);
}

/*
  'DOCUMENT_URI' => '/render-document.php/racer/CarTagsDocument',
  'SCRIPT_NAME' => '/render-document.php/racer/CarTagsDocument',
  'SCRIPT_FILENAME' => '/var/www/html/render-document.php',
  'PATH_TRANSLATED' => '/var/www/html',
  'PHP_SELF' => '/render-document.php/racer/CarTagsDocument',
  'DOCUMENT_ROOT' => '/var/www/html',
*/
 if (!$have_args && isset($_SERVER['PHP_SELF']) && isset($_SERVER['DOCUMENT_ROOT']) &&
     isset($_SERVER['SCRIPT_FILENAME']) &&
     substr($_SERVER['SCRIPT_FILENAME'], 0, strlen($_SERVER['DOCUMENT_ROOT'])) == $_SERVER['DOCUMENT_ROOT'] &&
     substr($_SERVER['PHP_SELF'], 0, strlen($_SERVER['SCRIPT_FILENAME']) - strlen($_SERVER['DOCUMENT_ROOT'])) ==
     substr($_SERVER['SCRIPT_FILENAME'],  strlen($_SERVER['DOCUMENT_ROOT']))) {
   $args = substr($_SERVER['PHP_SELF'], strlen($_SERVER['SCRIPT_FILENAME']) - strlen($_SERVER['DOCUMENT_ROOT']));
   $have_args = confirm_args($args);
}

if (!$have_args) {
  // echo "Debugging \$_SERVER:\n";
  // var_export($_SERVER);
  exit(0);
}

$exploded = explode('/', $args);

while ($exploded[0] == '') {
  array_shift($exploded);
}

$inc = array_shift($exploded);

function document_class() {
  global $exploded;
  return $exploded[0];
}

function new_document() {
  $doc_class = document_class();
  return new $doc_class;
}

// Use windows-1252 encodings in the PDF to display correctly
function convert($s) {
  return iconv('UTF-8', 'windows-1252', $s);
}

function convert_strings(&$row) {
  foreach ($row as $key => $value) {
    if (is_string($value)) {
      $row[$key] = iconv('UTF-8', 'windows-1252', $value);
    }
  }
}

function clean_fake_photos(&$row) {
  // "Fake" roster includes svg files, but fpdf doesn't support them
  if (isset($row['imagefile']) && substr($row['imagefile'], -4) == '.svg') {
    $row['imagefile'] = '';
  }
  if (isset($row['carphoto']) && substr($row['carphoto'], -4) == '.svg') {
    $row['carphoto'] = '';
  }
}

require_once('print/render/'.$inc.'.inc');
?>
