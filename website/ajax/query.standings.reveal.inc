<?php
require_once('inc/standings.inc');

$state = explode('-', read_raceinfo('standings-message'));
if (count($state) < 2) {
  echo "<standings-reveal/>\n";
  exit(0);
}

if (count($state) == 2) {  // Backwards compatibility
  $roundid = $state[0];  // empty for whole-pack standings
  $rankid = '';
  $exposed = $state[1];  // empty for show-all-standings
} else {
  $roundid = $state[0];  // empty for whole-pack standings
  $rankid = $state[1];
  $exposed = $state[2];  // empty for show-all-standings
}

?><standings-reveal>
  <round roundid="<?php echo $roundid; ?>" rankid="<?php echo $rankid; ?>"><?php
      if ($roundid == "") {
        echo htmlspecialchars(supergroup_label(), ENT_QUOTES, 'UTF-8');
      } else {
        $rounds = rounds_for_standings($roundid);
        if ($rankid == "") {
          echo htmlspecialchars($rounds[0]['name'], ENT_QUOTES, 'UTF-8');
        } else {
          foreach ($rounds[0]['ranks'] as $rank) {
            if ($rank['rankid'] == $rankid) {
              echo htmlspecialchars($rank['name'], ENT_QUOTES, 'UTF-8');
            }
          }
        }
      }
  ?></round>
  <exposed count="<?php echo $exposed; ?>"/>
</standings-reveal>
