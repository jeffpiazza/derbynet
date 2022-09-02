<?php

require_once('inc/name-mangler.inc');
require_once('inc/path-info.inc');
require_once('inc/photo-config.inc');
require_once('inc/schema_version.inc');

function url_for_cropped(&$row, $column_name, &$repository) {
  return $repository->url_for_file($row[$column_name], $row['racerid'], RENDER_CROPPED);
}


$done = false;
$mode = 'title';
if (isset($_GET['mode'])) {
  $mode = $_GET['mode'];
}

$racerid = 0;
if (isset($_GET['racerid'])) {
  $racerid = $_GET['racerid'];
}

$classids = '';
if (isset($_GET['classids'])) {
  $classids = $_GET['classids'];
}


if ($mode == 'slide') {
  $f = $_GET['file'];

  // TODO realpath
  $results = array();

  // Title slide could come from the local override directory or the selected
  // stock directory.  Other slides can only come from local override directory.
  // Look up the title slide only to exclude it as a base name.
  $title = basename(slide_file_path('title'));
  if (isset($homedir)) {
    $dir = $homedir.DIRECTORY_SEPARATOR.'slides' /* TODO .DIRECTORY_SEPARATOR */;
    if (is_dir($dir) && is_readable($dir)) {
      if ($dh = opendir($dir)) {
        while (($file = readdir($dh)) !== false) {
          if ($file > $f && $file != "." && $file != ".." && $file != $title) {
            $results[] = $file;
          }
        }
      }
      closedir($dh);
      if (count($results) > 0) {
        sort($results);
        json_out('photo', array('photo' => 'slide.php/'.pathinfo($results[0])['filename'],
                                'next' => array('mode' => 'slide',
                                                'file' => $results[0])));
        $done = true;
      }
    }
  }

  $mode = 'racer';
}

if (!$done && $mode == 'racer') {
  $row = read_single_row('SELECT racerid, lastname, firstname, carnumber, carname, classid,'
                         .' imagefile, carphoto FROM RegistrationInfo'
                         .' WHERE racerid > :racerid'
                         .'  AND ((imagefile IS NOT NULL AND imagefile <> \'\') OR'
                         .'       (carphoto IS NOT NULL AND carphoto <> \'\'))'
                         .($classids
                           ? ' AND classid IN ('.$classids.')'
                           : '')
                         .' ORDER BY racerid',
                         array(':racerid' => $racerid),
                         PDO::FETCH_ASSOC);
  if ($row !== false) {
    $out = array('name' => mangled_name($row, read_name_style()),
                 'carnumber' => $row['carnumber'],
                 'carname' => $row['carname'],
                 'next' => array('mode' => 'racer',
                                 'racerid' => $row['racerid']));
    if (isset($row['carphoto'])) {
      $out['photo'] = url_for_cropped($row, 'carphoto', car_photo_repository());
      if (isset($row['imagefile'])) {
        $out['inset'] = url_for_cropped($row, 'imagefile', headshots());
      }
    } else {
      // From the query, imagefile should be populated if carphoto isn't
      $out['photo'] = url_for_cropped($row, 'imagefile', headshots());
    }
    json_out('photo', $out);
    $done = true;
  }
  $mode = 'title';
}

if (!$done && $mode == 'title') {
  json_out('photo', array('photo' => 'slide.php/title',
                          'title' => true,
                          'next' => array('mode' => 'slide',
                                          'file' => '')));
  $done = true;
}

?>
