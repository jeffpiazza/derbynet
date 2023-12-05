<?php

// replay_last_contact

// replay_state:
define('REPLAY_NOT_CONNECTED', 1);
define('REPLAY_CONNECTED', 2);  // See replay_last_contact.
define('REPLAY_TROUBLE', 3);   // See replay_trouble value

// replay_reported - see $status_array, below.
define('REPLAY_REPORTED_PLAYING', 2);

// A registration request causes a HELLO message to be sent, and the state
// becomes either CONNECTED or TROUBLE depending on the outcome.

// replay_last_contact gets updated with succesful message sent.  replay_trouble
// gets set to the error message for an unsuccessful message send.

// REPLAY_CONNECTED or REPLAY_TROUBLE become REPLAY_NOT_CONNECTED if
// REPLAY_TIMEOUT_LIMIT passes since replay_last_contact.
//
// REPLAY_TROUBLE causes message sends to be ignored (except re-registration
// attempts).  Re-registration attempt also sets replay_last_contact.


require_once('inc/data.inc');

define('REPLAY_TIMEOUT_LIMIT', 10);

function get_replay() {
  $state = read_raceinfo('replay_state', REPLAY_NOT_CONNECTED);
  $reported_status = read_raceinfo('replay_reported', 0);
  $last_contact = read_raceinfo('replay_last_contact', 0);
  if ($last_contact + REPLAY_TIMEOUT_LIMIT < time()) {
    $state = REPLAY_NOT_CONNECTED;
  }
  $trouble = read_raceinfo('replay_trouble');

  $status_array = array(
         0 => 'Ready',
         1 => 'Recording',
         2 => 'Playing',
         -1 => 'Connecting...',
         -2 => 'No video source chosen',
         -3 => 'No audio source chosen',
         -4 => 'Recording error');

  return array('state' => $state,
               'status' => $reported_status,
               'reported_status' => $status_array[$reported_status],
               'last_contact' => $last_contact,
               'trouble' => $trouble);
}

function send_replay_message($cmd) {
  global $homedir;
  global $db;
  $replay = get_replay();
  if ($replay['state'] == REPLAY_CONNECTED) {
    if (isset($homedir) && is_dir($homedir) && is_writable($homedir)) {
      $fp = fopen($homedir.DIRECTORY_SEPARATOR.'replay_queue', 'c+');
      try {
        if (flock($fp, LOCK_EX)) {
          if (fseek($fp, 0, SEEK_END) == 0) {
            fwrite($fp, $cmd."\n");
          }
        }
      } finally {
        fclose($fp);
      }
    } else {
      // TODO: The trouble here is that the RaceInfo table's ItemValue may not be
      // long enough for what we need, including file names
      try {
        if ($db->beginTransaction()) {
          try {
            write_raceinfo('replay_queue', read_raceinfo('replay_queue', '').$cmd."\n");
            $db->commit();
          } catch (PDOException $e) {
            $db->rollBack();
            error_log("send_replay_message: transaction rolled back: ".$e->getMessage());
          }
        } else {
          error_log("send_replay_message: beginTransaction returns false");
        }
      } catch (PDOException $e) {
        error_log("send_replay_message: beginTransaction throws ".$e->getMessage());
      }
      try {
        $db->setAttribute(PDO::ATTR_AUTOCOMMIT, true);
      } catch (PDOException $e) {
        error_log("send_replay_message: set autocommit throws ".$e->getMessage());
      }
    }
    return true;
  } else {
    return false;
  }
}

function drain_queue() {
  global $homedir;
  global $db;
  $queue = '';
  if (isset($homedir) && is_dir($homedir) && is_writable($homedir)) {
    $fp = fopen($homedir.DIRECTORY_SEPARATOR.'replay_queue', 'c+');
    try {
      if (flock($fp, LOCK_EX)) {
        $queue = stream_get_contents($fp);
        ftruncate($fp, 0);
      }
    } finally {
      fclose($fp);
    }
  } else {
    $db->beginTransaction();
    $queue = read_raceinfo('replay_queue');
    write_raceinfo('replay_queue', '');
    $db->commit();
    try {
      $db->setAttribute(PDO::ATTR_AUTOCOMMIT, true);
    } catch (PDOException $p) {
    }
  }

  return array_filter(explode("\n", $queue), 'not_empty_string');
}

function send_replay_TEST() {
  // test <skipback_seconds> <repeat> <rate>
  $skipback = read_raceinfo('replay-skipback', '4');
  $num_showings = read_raceinfo('replay-num-showings', '2');
  $rate = read_raceinfo('replay-rate', '0.5');
  send_replay_message("TEST ".$skipback." ".$num_showings." ".$rate);
}

// "START video_name_root  -- start recording video\r\n"
function send_replay_START(&$current) {
  send_replay_message("START ".@$current['class']
                      .'_Round'.@$current['round']
                      .'_Heat'.sprintf("%02d", @$current['heat']));
}

// "REPLAY skipback showings rate  -- stop recording if recording; playback\r\n"
function send_replay_REPLAY() {
  $skipback = read_raceinfo('replay-skipback', '4');
  $num_showings = read_raceinfo('replay-num-showings', '2');
  $rate = read_raceinfo('replay-rate', '0.5');
  send_replay_message("REPLAY ".$skipback." ".$num_showings." ".$rate);
}

// "CANCEL  -- cancel recording\r\n"
function send_replay_CANCEL() {
  send_replay_message("CANCEL");
}

// RACE_STARTS skipback showings rate -- induce a replay in replay-skipback less epsilon
function send_replay_RACE_STARTS() {
  $skipback = read_raceinfo('replay-skipback', '4');
  $num_showings = read_raceinfo('replay-num-showings', '2');
  $rate = read_raceinfo('replay-rate', '0.5');
  send_replay_message("RACE_STARTS $skipback $num_showings $rate");
}

?>
