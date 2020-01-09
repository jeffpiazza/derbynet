<?php

function xbs_award() {
  if (!read_raceinfo_boolean('use-xbs')) {
    return '';
  }
  return read_raceinfo('xbs-award');
}

?>