<?php

require_once('inc/sql-script.inc');

// If we're working with an unmigrated GPRM database, we may need to create a
// kiosk table.
function create_kiosk_table() {
  run_sql_script('kiosk-table');
}

// Compute a string that acts as the "address" for the kiosk that invoked the
// current page.
function address_for_current_kiosk() {
  // The Kiosks table in the database has a field for the address, and another
  // for the name.  Both are displayed to the race coordinator.  The address is
  // what identifies the kiosk; the name is just a convenience for the user.
  //
  // The kiosk browser can include an 'id' or 'address' parameter which allows
  // an arbitrary string to be used as the address identifier.
  $addr = "";
  if (isset($_GET['id'])) {
    $addr = $_GET['id'];
  } else if (isset($_GET['address'])) {
    $addr = $_GET['address'];
  } else if (isset($_SESSION['address'])) {
    $addr = $_SESSION['address'];
  }
  if ($addr === "") {
    if (isset($_SERVER['REMOTE_PORT'])) {
      $port = $_SERVER['REMOTE_PORT'];
    } else {
      $port = rand(100, 200);
    }
    $addr = $_SERVER['REMOTE_ADDR'].'+'.$port;
    $_SESSION['address'] = $addr;
  }
  return $addr;
}

function madlib_string($addr) {
  $adjs = ['abundant', 'delightful', 'high', 'nutritious', 'square',
              'adorable', 'dirty', 'hollow', 'obedient', 'steep', 'agreeable',
              'drab', 'hot', 'obnoxious', 'sticky', 'alive', 'dry', 'hot',
              'odd', 'straight', 'angry', 'dusty', 'huge', 'old-fashioned',
              'strong', 'beautiful', 'eager', 'icy', 'orange', 'substantial',
              'better', 'early', 'immense', 'panicky', 'sweet', 'bewildered',
              'easy', 'important', 'petite', 'swift', 'big', 'elegant',
              'inexpensive', 'plain', 'tall', 'bitter', 'embarrassed', 'itchy',
              'powerful', 'tart', 'black', 'empty', 'jealous', 'prickly',
              'tasteless', 'blue', 'faint', 'jolly', 'proud', 'teeny',
              'boiling', 'faithful', 'juicy', 'puny', 'teeny-tiny', 'brave',
              'famous', 'kind', 'purple', 'tender', 'breeze', 'fancy', 'large',
              'purring', 'thankful', 'broad', 'fast', 'late', 'quaint',
              'thoughtless', 'broken', 'fat', 'lazy', 'quick', 'tiny', 'bumpy',
              'few', 'light', 'quiet', 'ugliest', 'calm', 'fierce', 'little',
              'rainy', 'ugly', 'careful', 'filthy', 'lively', 'rapid', 'uneven',
              'chilly', 'flaky', 'long', 'red', 'uninterested', 'chubby',
              'flat', 'loose', 'relieved', 'unsightly', 'clean', 'fluffy',
              'loud', 'repulsive', 'uptight', 'clever', 'freezing', 'low',
              'rich', 'vast', 'clumsy', 'fresh', 'lumpy', 'rotten',
              'victorious', 'cold', 'full', 'magnificent', 'round', 'warm',
              'colossal', 'gentle', 'mammoth', 'salty', 'weak', 'cool',
              'gifted', 'many', 'scary', 'wet', 'creepy', 'gigantic', 'massive',
              'scrawny', 'west', 'crooked', 'glamorous', 'melted', 'screeching',
              'whispering', 'cuddly', 'gray', 'messy', 'shallow', 'white',
              'curly', 'greasy', 'miniature', 'short', 'wide', 'curved',
              'great', 'modern', 'shy', 'wide-eyed', 'damaged', 'green',
              'mushy', 'silly', 'witty', 'damp', 'grumpy', 'mysterious',
              'skinny', 'wooden', 'dead', 'handsome', 'narrow', 'slow',
              'worried', 'deafening', 'happy', 'nervous', 'small', 'wrong',
              'deep', 'heavy', 'nice', 'soft', 'yellow', 'defeated', 'helpful',
              'noisy', 'sparkling', 'young', 'delicious', 'helpless',
              'numerous', 'sparse', 'yummy'];

  $nouns = ['apple', 'fifth', 'scale', 'arithmetic', 'finger', 'seashore',
               'badge', 'flock', 'sidewalk', 'basket', 'frame', 'sleet',
               'basketball', 'furniture', 'smoke', 'battle', 'geese', 'bathtub',
               'beast', 'ghost', 'nose', 'beetle', 'giraffe', 'sidewalk',
               'beggar', 'governor', 'boy', 'brain', 'honey', 'stage', 'bubble',
               'hope', 'station', 'bucket', 'income', 'throat', 'cactus',
               'island', 'throne', 'cannon', 'jeans', 'title', 'cow', 'judge',
               'toothbrush', 'celery', 'lamp', 'turkey', 'cellar', 'lettuce',
               'umbrella', 'cloth', 'marble', 'underwear', 'coach', 'month',
               'vacation', 'coast', 'north', 'south', 'east', 'west',
               'vegetable', 'crate', 'ocean', 'poison', 'cream', 'patch',
               'riddle', 'girl', 'plane', 'scale', 'donkey', 'playground',
               'visitor', 'earthquake', 'poison', 'voyage', 'feast', 'riddle',
               'year'];

  $crc = abs(crc32($addr));
  return $adjs[$crc % count($adjs)]." ".$nouns[($crc >> 16) % count($nouns)];
}

// Splits $row['params'] into separate ['page'] and ['params'] fields.
function parse_kiosk_params(&$row) {
  $ex = explode('#', $row['page'], 2);
  if (count($ex) > 1) {
    $row['page'] = $ex[0];
    $row['params'] = $ex[1];
  } else {
    $row['params'] = '';
  }
}

// Searches the Kiosks table, if there is one, for an entry for the given kiosk
// address.  If there's a page assigned for this client, returns that.
// Otherwise, returns a RaceInfo entry for the default kiosk page, if there is
// one (there won't be: obsolete feature); otherwise, returns the identify.kiosk
// page.
//
// Returns an array with a 'page' key and optionally 'name' (of the kiosk) and
// 'params' (a customization string).
function kiosk_page($addr) {
  global $db;
  do {  // Retry loop to allow for having to create the table
    $retry = false;
    try {
      $stmt = $db->prepare('SELECT page, name FROM Kiosks WHERE address = :address');
      $stmt->execute(array(':address' => $addr));
      $row = $stmt->fetch(PDO::FETCH_ASSOC);
      $stmt->closeCursor();
      if ($row) {
        $row['reload'] = false;
        if (!isset($row['name'])) {
          $row['name'] = '';
        }
        if (@$row['page'][0] == '!') {
          $row['page'] = substr($row['page'], 1);
          $row['reload'] = true;
          $stmt = $db->prepare('UPDATE Kiosks SET last_contact = '.time()
                               .', page = :page'
                               .' WHERE address = :address');
          $stmt->execute(array(':address' => $addr,
                               ':page' => $row['page']));
        } else {
          $stmt = $db->prepare('UPDATE Kiosks SET last_contact = '.time()
                               .' WHERE address = :address');
          $stmt->execute(array(':address' => $addr));
        }
        if (@$row['page']) {
          parse_kiosk_params($row);
          return $row;
        }
        // else row exists, but no page set
      } else {
        // No row exists
        $stmt = $db->prepare('INSERT INTO Kiosks(address, page, last_contact)'
                             .' VALUES(:address, :page, '.time().')');
        $stmt->execute(array(':address' => $addr,
                             ':page' => 'kiosks'.DIRECTORY_SEPARATOR.'identify.kiosk'));
      }
    } catch (PDOException $p) {
      if (is_no_such_table_exception($p)) {
        create_kiosk_table();
        $retry = true;
      } else {
        echo "SQL error code ".$p->getCode()."\n";
        echo $p;
      }
    }
  } while ($retry);
  // Return global kiosk-page RaceInfo setting, or identify.kiosk.
  return array('page' => read_raceinfo('kiosk-page', 'kiosks'.DIRECTORY_SEPARATOR.'identify.kiosk'),
               'params' => '',
               'reload' => false);
}

function mark_kiosk_for_reload($addr) {
  global $db;
  try {
    $stmt = $db->prepare('UPDATE Kiosks SET page = \'!\' || page'
                         .' WHERE address = :address'
                         .' AND SUBSTR(page, 1, 1) <> \'!\'');
    $stmt->execute(array(':address' => $addr));
  } catch (PDOException $p) {
  }
}

function mark_all_kiosks_for_reload() {
  global $db;
  try {
    $stmt = $db->prepare('UPDATE Kiosks SET page = \'!\' || page'
                         .' WHERE SUBSTR(page, 1, 1) <> \'!\'');
    $stmt->execute();
  } catch (PDOException $p) {
  }
}

// Returns an array of {address, name, page, parameters, last_contact}
function all_kiosks() {
  global $db;
  $kiosks = array();
  try {
    drop_old_kiosks();
    // Information for "old" kiosks remains in the database but we filter out
    // those we haven't heard from in 60 seconds.
    $stmt = $db->query('SELECT address, name, page, last_contact'
                       .' FROM Kiosks'
                       .' WHERE '.time().' - last_contact < 60'
                       .' ORDER BY '
                       .'     CASE name WHEN \'Main\' THEN \'\' ELSE name END,'
                       .'        address, last_contact');
    foreach ($stmt as $row) {
      if (@$row['page'][0] == '!') {
        $row['page'] = substr($row['page'], 1);
        $row['reload'] = true;
      } else {
        $row['reload'] = false;
      }
      parse_kiosk_params($row);
      $kiosks[] = array('name' => $row['name'] ? $row['name'] : '',
                        'address' => $row['address'],
                        'madlib' => madlib_string($row['address']),
                        'last_contact' => $row['last_contact'],
                        'page' => $row['page'],
                        'reload' => $row['reload'],
                        'age' => time() - $row['last_contact'],
                        'parameters' => $row['params']);
    }
  } catch (PDOException $p) {
    // Most likely: no kiosks table.
    // One will be created if a kiosk actually connects.
  }
  return $kiosks;
}

function all_kiosk_pages_with_prefix($prefix) {
  $entries = array();
  if (is_dir($prefix)) {
    $dh = opendir($prefix);
    if ($dh !== false) {
      while (($entry = readdir($dh)) !== FALSE) {
        if (substr($entry, -6) == ".kiosk") {
          $brief = substr($entry, 0, -6);
          $entries[] = array('brief' => $brief,
                             'full' => $prefix.DIRECTORY_SEPARATOR.$entry);
        }
      }
      closedir($dh);
    }
  }

  sort($entries);

  return $entries;
}

function remove_prefixes($prefix, $entries) {
  $prefix_length = strlen($prefix);
  foreach ($entries as &$entry) {
    if (substr_compare($entry['full'], $prefix, 0, $prefix_length) == 0) {
      $entry['full'] = substr($entry['full'], $prefix_length);
    }
  }
  return $entries;
}

function all_kiosk_pages() {
  $configdir = isset($_SERVER['DERBYNET_CONFIG_DIR']) ? $_SERVER['DERBYNET_CONFIG_DIR'] : 'local';
  return remove_prefixes(dirname($_SERVER['SCRIPT_FILENAME']).DIRECTORY_SEPARATOR,
                         array_merge(all_kiosk_pages_with_prefix(
                                       dirname($_SERVER['SCRIPT_FILENAME']).DIRECTORY_SEPARATOR.'kiosks'),
                                     all_kiosk_pages_with_prefix($configdir.DIRECTORY_SEPARATOR.'kiosks')));
}

// Kiosk panes that don't have focus may not check in as often as expected.
// This function deletes "old" kiosks that haven't made contact for an hour
// (3600 seconds) -- mainly this would be kiosks from testing that we don't want
// to remain in the database on race day.
//
// Note that all_kiosks(), above, separately filters out kiosks that haven't
// made contact much more recently.
function drop_old_kiosks() {
  global $db;
  $stmt = $db->query('DELETE FROM Kiosks WHERE '.time().' - last_contact > 3600');
}
?>
