<?php
require_once('inc/export-roster.inc');
require_once('inc/export-results.inc');
require_once('inc/export-standings.inc');
require_once('inc/export-awards.inc');

// Returns an array of workbook pages, as consumed by XLSX javascript.
function export_all() {
  $workbook = array();

  ////////////// Roster
  $roster = array();
  export_roster(function($row) use (&$roster) { $roster[] = $row; });
  $workbook[] = array('Roster', $roster);

  ////////////// Results
  $results = array();
  export_results(function($row) use (&$results) { $results[] = $row; });
  $workbook[] = array('Results', $results);

  ////////////// Standings
  $use_subgroups = use_subgroups();

  $standings = new StandingsOracle();
  foreach ($standings->standings_catalog() as $entry) {
    $title = 'Standings '.preg_replace('+[/]+', '-', $entry['name']);
    // XLSX won't allow sheet name to exceed 31 characters
    if (strlen($title) > 31) {
      $title = 'Standings '.$entry['key'];
    }
    if ($entry['presentation'] == 'ff') {
      $title = 'Standings';
    }

    $sheet = array();
    $sheet[] = array('Standings for '.$entry['name']);
    export_standings(function($row) use (&$sheet) { $sheet[] = $row; },
                     $entry['key'], $entry['presentation'], $standings->result_summary);
    $workbook[] = array($title, $sheet);
  }

  ////////////// Awards
  $awards = array();
  export_awards(function($row) use (&$awards) { $awards[] = $row; });
  $workbook[] = array('Awards', $awards);

  // Javascript XLSX doesn't allow sheet names to contain : \ / ? * [ ], or to
  // exceed 31 characters
  foreach ($workbook as $w) {
    $w[0] = str_replace(array('/', '\\', '?', '*', '[', ']'), '-', $w[0]);
    if (strlen($w[0]) > 31) {
      $w[0] = substr($w[0], 0, 31);
    }
  }

  return $workbook;
}

?>