<?php

require_once('inc/name-mangler.inc');
require_once('inc/path-info.inc');
require_once('inc/photo-config.inc');
require_once('inc/schema_version.inc');

function url_for_cropped(&$row, $column_name, &$repository) {
  return $repository->url_for_file($row[$column_name], $row['racerid'], RENDER_CROPPED);
}


$done = false;

// The three "modes" for the slide show are 'title', 'slide', and 'racer'.
// 'title' isn't really a mode that the client would ask for, but it's a useful
// way to represent a request for the title slide.  'slide' mode explicitly
// assumes the client wants slides other than the title slide, and 'racer' mode
// is looking for racer photos.
//
// A 'title' mode query would always produce a title slide.  A 'slide' mode
// request tries to give the next slide alphabetically after the 'file'
// argument; if there isn't one, then we advance to 'racer' mode.  'racer' mode
// tries to return information about the next racer in racerid order; if there
// isn't one, then we cycle back around to 'title' mode and give a title slide.
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

  $results = next_slides($f);
  if (count($results) > 0) {
    json_out('debug-results', $results);
    json_out('photo', array('photo' => 'slide.php/'.$results[0],
                            'next' => array('mode' => 'slide',
                                            'file' => $results[0])));
    $done = true;
  }

  $mode = 'racer';
  $racerid = 0;
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
