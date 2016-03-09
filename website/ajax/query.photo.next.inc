<slideshow>
<?php

// Returns photo information for the next racer with a photo

$racerid = 0;
if (isset($_GET['racerid'])) {
  $racerid = $_GET['racerid'];
}

require_once('inc/photo-config.inc');
require_once('inc/schema_version.inc');

// TODO Any special order we prefer to racerid?
// TODO Slideshows by subset, e.g., by den?

// If working with a GPRM database, the 'imagefile' field carries the only photo.
// If working with a DerbyNet database, 'imagefile' field carries the inset photo, and 'carphoto' is the main photo.
$inset_imagefile = schema_version() >= 2;

$main_repository = $inset_imagefile ? car_photo_repository() : headshots();
$inset_repository = headshots();

$stmt = $db->prepare('SELECT racerid, lastname, firstname, carnumber, carname,'
                     .($inset_imagefile
                       ? ' imagefile as inset_photo, carphoto as main_photo'
                       : ' imagefile as main_photo')
                     .' FROM RegistrationInfo'
                     .' WHERE racerid > :racerid'
                     .'  AND main_photo IS NOT NULL'
                     .'  AND main_photo <> \'\''
                     .' ORDER BY racerid');
$stmt->execute(array(':racerid' => $racerid));

$row = $stmt->fetch();
if ($row) {
    echo '<racer';
    echo ' racerid="'.$row['racerid'].'"';
    echo ' lastname="'.htmlspecialchars($row['lastname'], ENT_QUOTES, 'UTF-8').'"';
    echo ' firstname="'.htmlspecialchars($row['firstname'], ENT_QUOTES, 'UTF-8').'"';
    echo ' carnumber="'.htmlspecialchars($row['carnumber'], ENT_QUOTES, 'UTF-8').'"';
    echo ' carname="'.htmlspecialchars($row['carname'], ENT_QUOTES, 'UTF-8').'"';
    
    if (isset($row['inset_photo']) && $row['inset_photo']) {
      echo ' inset_photo="'.$inset_repository->url_for_file($row['inset_photo'], $row['racerid'], 'cropped').'"';
    }
    echo ' main_photo="'.$main_repository->url_for_file($row['main_photo'], $row['racerid'], 'cropped').'"';
    $row = $stmt->fetch();
    if ($row) {
      echo ' next_photo="'.$main_repository->url_for_file($row['main_photo'], $row['racerid'], 'cropped').'"';
    }
    echo '/>'."\n";
}
// If that's the last racer, return no <racer/> element, and let the client send
// another requrest to start from 0.
?>
</slideshow>