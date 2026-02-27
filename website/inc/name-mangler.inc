<?php

define('FULL_NAME', 0);
define('FIRST_NAME_LAST_INITIAL', 1);
define('PHONETIC_NAME', 2);

function read_name_style() {
  return read_raceinfo('name-style', FULL_NAME);
}

function mangled_name(&$racer, $style) {
  if ($style == FIRST_NAME_LAST_INITIAL) {
    return $racer['firstname'].' '.substr($racer['lastname'], 0, 1).'.';
  } else if ($style == PHONETIC_NAME) {
    // Use phonetic names if available, fall back to regular names
    $firstname = !empty($racer['phonetic_firstname']) ? $racer['phonetic_firstname'] : $racer['firstname'];
    $lastname = !empty($racer['phonetic_lastname']) ? $racer['phonetic_lastname'] : $racer['lastname'];
    return $firstname.' '.$lastname;
  } else {
    return $racer['firstname'].' '.$racer['lastname'];
  }
}
?>
