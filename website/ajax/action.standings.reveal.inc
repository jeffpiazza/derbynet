<?php

// $_POST['catalog-entry'] unset for "no change"
// $_POST['catalog-entry'] JSON string for a new standings_catalog entry
//
// $_POST['expose'] unset for "show NONE", i.e., same as 0
// $_POST['expose'] = n or +n or 'all'
//
//
// standings-message value is <exposed>-<catalog-entry>
//  where <exposed> is a number or '' (all shown), and
//        <catalog-entry> is json for the standings_catalog() entry.
//
//
// <exposed> == '' means all shown

start_response();

if (have_permission(PRESENT_AWARDS_PERMISSION)) {
  $prev = explode('-', read_raceinfo('standings-message', '0-'), 2);
  $expose = $prev[0];
  $entry = $prev[1];
  if (isset($_POST['catalog-entry']) && !empty($_POST['catalog-entry'])) {
    $e = json_decode($_POST['catalog-entry'], /* assoc */ true);
    if (isset($e['key'])) {
      $entry = json_encode($e);
    }
  }

  if (!isset($_POST['expose']) or $_POST['expose'] == '') {
    $expose = '0';
  } else if ($_POST['expose'] == 'all') {
    $expose = '';
  } else if (substr($_POST['expose'], 0, 1) == '+') {
    if ($expose != '') {
      $expose = intval(substr($_POST['expose'], 1)) + intval($expose);
    }
  } else {
    $expose = intval($_POST['expose']);
  }

  if (!empty($entry)) {
    write_raceinfo('standings-message', $expose.'-'.$entry);
    echo "<reveal count=\"$expose\"/>\n";
  }
  echo "<success/>\n";
} else {
  not_authorized_body();
}

end_response();
?>