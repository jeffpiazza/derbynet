<?php
require_once('inc/standings.inc');

$state = explode('-', read_raceinfo('standings-message', ''), 2);
if (count($state) == 2) {
  $exposed = $state[0];
  $entry = $state[1];

  json_out('catalog-entry', json_decode($entry));
  json_out('exposed', $exposed);
}

?>