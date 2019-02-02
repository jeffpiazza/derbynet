<?php
require_once('inc/data.inc');

define(PHOTOS_ON_NOW_RACING, 'photos-on-now-racing');

function read_photos_on_now_racing() {
  global $db;
  $photos_on_now_racing = read_raceinfo(PHOTOS_ON_NOW_RACING);
  if (!$photos_on_now_racing) {
    $show_racer_photos = read_raceinfo_boolean('show-racer-photos');
    $photos_on_now_racing = $show_racer_photos ? 'head' : '0';
    write_raceinfo(PHOTOS_ON_NOW_RACING, $photos_on_now_racing);
    $db->exec("DELETE FROM RaceInfo WHERE itemkey = 'show-racer-photos'");
  }
  return $photos_on_now_racing;
}

?>