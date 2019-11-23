<?php
require_once('inc/standings.inc');

$state = explode('-', read_raceinfo('standings-message'), 2);
if (count($state) < 2) {
  echo "<standings-reveal/>\n";
  exit(0);
}

$exposed = $state[0];
$entry = $state[1];

?><standings-reveal>
  <catalog-entry json="<?php echo htmlspecialchars($entry, ENT_QUOTES, 'UTF-8'); ?>"/>
  <exposed count="<?php echo $exposed; ?>"/>
</standings-reveal>
