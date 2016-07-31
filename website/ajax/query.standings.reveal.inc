<?php
require_once('inc/standings.inc');

$state = explode('-', read_raceinfo('standings-message'));
$roundid = $state[0];  // empty for whole-pack standings
$exposed = $state[1];  // empty for show-all-standings

if (count($state) < 2) {
  echo "<standings-reveal/>\n";
  exit(0);
}

?><standings-reveal>
  <round roundid="<?php echo $roundid; ?>"><?php
      if ($roundid == "") {
        echo htmlspecialchars(supergroup_label(), ENT_QUOTES, 'UTF-8');
      } else {
        $classnames = standings_round_names($roundid);
        echo htmlspecialchars($classnames[0]['name'], ENT_QUOTES, 'UTF-8');
      }
  ?></round>
  <exposed count="<?php echo $exposed; ?>"/>
</standings-reveal>
