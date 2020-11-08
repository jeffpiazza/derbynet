<?php

require_once('inc/voterid.inc');

echo "<ballot-results>\n";

$tab = vote_tabulations();

foreach ($tab as $award) {
  echo "<award awardid=\"$award[awardid]\""
      ." awardname=\"".htmlspecialchars($award['awardname'], ENT_QUOTES, 'UTF-8')."\""
      ." recipient=\"$award[recipient]\""
      .">";
  foreach ($award['candidates'] as $can) {
    echo "<candidate score=\"$can[score]\""
        ." carnumber=\"$can[carnumber]\">";
    echo htmlspecialchars($can['firstname'].' '.$can['lastname'], ENT_QUOTES, 'UTF-8');
    echo "</candidate>";
  }
  echo "</award>\n";
}

echo "</ballot-results>\n";
?>