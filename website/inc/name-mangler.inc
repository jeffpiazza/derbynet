<?php

define('FULL_NAME', 0);
define('FIRST_NAME_LAST_INITIAL', 1);

function read_name_style() {
  return read_raceinfo('name-style', FULL_NAME);
}

function mangled_name(&$racer, $style) {
  if ($style == FIRST_NAME_LAST_INITIAL) {
    return $racer['firstname'].' '.substr($racer['lastname'], 0, 1).'.';
  } else {
    return $racer['firstname'].' '.$racer['lastname'];
  }
}
?>