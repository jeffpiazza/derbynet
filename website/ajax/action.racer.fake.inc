<?php

// $_POST['ngroups']
// $_POST['racers_per_group']
// $_POST['check_in']

require_once('inc/newracer.inc');
require_once('inc/sql-script.inc');
require_once(sql_file_path('column-widths'));
require_once('inc/schema_version.inc');
require_once('inc/fake_names.inc');
require_once('inc/photo-config.inc');

define('FAKE_RACER_IMAGES', 'fake-racer-images/racers');
define('FAKE_CAR_IMAGES', 'fake-racer-images/cars');

function fake_partition_name($g) {
  if ($g == 1) return 'Fake Lions';
  if ($g == 2) return 'Fake Tigers';
  if ($g == 3) return 'Fake Wolves';
  if ($g == 4) return 'Fake Bears';
  if ($g == 5) return 'Fake Webelos';
  return 'Fake Group '.$g;
}

if (have_permission(SET_UP_PERMISSION)) {
  $target_per_group = $_POST['racers_per_group'];
  $fake_names = all_fake_names();
  $racer_photos = array_values(array_diff(scandir(FAKE_RACER_IMAGES), array('..', '.')));
  $car_photos = array_values(array_diff(scandir(FAKE_CAR_IMAGES), array('..', '.')));

  $swing = 2;

  for ($g = 1; $g <= $_POST['ngroups']; ++$g) {
    $group_name = fake_partition_name($g);
    $group_size = $_POST['racers_per_group'] + mt_rand(-$swing, $swing);
    for ($r = 0; $r < $group_size; ++$r) {
      $index = mt_rand(0, count($fake_names) - 1);
      $racer = $fake_names[$index];
      insert_new_racer(array('firstname' => $racer['firstname'],
                             'lastname' => $racer['lastname'],
                             'carname' => '',
                             'partition' => $group_name));

      $racerid = read_single_value('SELECT racerid FROM RegistrationInfo'
                                   .' WHERE firstname = :firstname'
                                   .' AND lastname = :lastname',
                                   array(':firstname' => $racer['firstname'],
                                         ':lastname' => $racer['lastname']));

      $racer_photo = photo_directory().DIRECTORY_SEPARATOR."fake_".$racerid.".svg";
      copy(FAKE_RACER_IMAGES.DIRECTORY_SEPARATOR.$racer_photos[mt_rand(0, count($racer_photos) - 1)],
           $racer_photo);
      $car_photo = car_photo_directory().DIRECTORY_SEPARATOR."fake_".$racerid.".svg";
      copy(FAKE_CAR_IMAGES.DIRECTORY_SEPARATOR.$car_photos[mt_rand(0, count($car_photos) - 1)], $car_photo);
      $stmt = $db->prepare('UPDATE RegistrationInfo'
                           .' SET imagefile = :racer_photo,'
                           .'     carphoto = :car_photo'
                           .' WHERE racerid = :racerid');
      $stmt->execute(array(':racer_photo' => $racer_photo,
                           ':car_photo' => $car_photo,
                           ':racerid' => $racerid));
      
      array_splice($fake_names, $index, 1);
      if (count($fake_names) == 0) {
        // TODO Having multiple racers with the exact same name is a problem.
        $fake_names = all_fake_names();
      }
    }
  }

  if ($_POST['check_in']) {
    $db->exec('UPDATE RegistrationInfo SET passedinspection = 1');
  }
  write_raceinfo('fake-racers', 1);

  json_success();
} else {
  json_not_authorized();
}
?>