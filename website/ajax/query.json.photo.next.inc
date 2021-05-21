<?php

require_once('inc/name-mangler.inc');

// Returns photo information for the next racer with a photo
//
// $_GET['racerid']: racer id for currently-displayed photo; we want the next racer's photo
//     If absent or zero, we want the first racer's photo.
// $_GET['classids']: If not empty, a comma-separated list of classids; we only want photos of racers from those classes.

$racerid = 0;
if (isset($_GET['racerid'])) {
  $racerid = $_GET['racerid'];
}

$classids = '';
if (isset($_GET['classids'])) {
  $classids = $_GET['classids'];
}

require_once('inc/photo-config.inc');
require_once('inc/schema_version.inc');

// TODO Any special order we prefer to racerid?

// Emits a url as the value for an xml attribute
function write_photo(&$row, $attribute_name, $column_name, &$repository) {
  echo ' '.$attribute_name.'="'.$repository->url_for_file($row[$column_name], $row['racerid'], RENDER_CROPPED).'"';
}

function write_main_photo(&$row, $attribute_name, &$repositories) {
  if ((isset($row['main_photo']) && $row['main_photo'])) {
    write_photo($row, $attribute_name, 'main_photo', $repositories[0]);
    return true;  // Continue on to inset
  } else if (isset($row['inset_photo']) && $row['inset_photo']) {
    write_photo($row, $attribute_name, 'inset_photo', $repositories[1]);
  }
  return false;  // Don't also inset the inset
}

$name_style = read_name_style();

// If working with a GPRM database, the 'imagefile' field carries the only photo.
// If working with a DerbyNet database, 'imagefile' field carries the inset photo, and 'carphoto' is the main photo.
$inset_imagefile = schema_version() >= 2;

$main_repository = $inset_imagefile ? car_photo_repository() : headshots();
$inset_repository = headshots();

$repositories = $inset_imagefile ? array(car_photo_repository(), headshots()) : array(headshots());

$stmt = $db->prepare('SELECT racerid, lastname, firstname, carnumber, carname, classid,'
                     .($inset_imagefile
                       ? ' imagefile as inset_photo, carphoto as main_photo'
                       : ' imagefile as main_photo')
                     .' FROM RegistrationInfo'
                     .' WHERE racerid > :racerid AND '
                     .($classids ? 'classid IN ('.$classids.') AND ' : '')
                     .'  ((imagefile IS NOT NULL AND imagefile <> \'\')'
                     .($inset_imagefile
                       ? '       OR (carphoto IS NOT NULL AND carphoto <> \'\')' : '')
                     .')'
                     .' ORDER BY racerid');
$stmt->execute(array(':racerid' => $racerid));

function url_for_cropped(&$row, $column_name, &$repository) {
  return $repository->url_for_file($row[$column_name], $row['racerid'], RENDER_CROPPED);
}

$row = $stmt->fetch();
if ($row) {
  $racer = array('racerid' => $row['racerid'],
                 'name' => mangled_name($row, $name_style),
                 'carnumber' => $row['carnumber'],
                 'carname' => $row['carname']);

  $write_inset = false;
  if (isset($row['main_photo']) && $row['main_photo']) {
    $racer['main-photo'] = url_for_cropped($row, 'main_photo', $repositories[0]);
    $write_inset = true;
  }

  if (isset($row['inset_photo']) && $row['inset_photo']) {
    $racer[$write_inset ? 'inset-photo' : 'main-photo'] =
        url_for_cropped($row, 'inset_photo', $repositories[1]);
  }

  $row = $stmt->fetch();
  if ($row) {
    $write_inset = false;
    if (isset($row['main_photo']) && $row['main_photo']) {
      $racer['next-photo'] = url_for_cropped($row, 'main_photo', $repositories[0]);
      $write_inset = true;
    }
    if (isset($row['inset_photo']) && $row['inset_photo']) {
      $racer[$write_inset ? 'next-inset' : 'next-photo'] =
               url_for_cropped($row, 'inset_photo', $repositories[1]);
    }
  }

  json_out('racer', $racer);
}
// If that's the last racer, return no racer, and let the client send
// another request to start from 0.
?>
