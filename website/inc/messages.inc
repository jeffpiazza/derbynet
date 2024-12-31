<?php

require_once('inc/schema_version.inc');
require_once('inc/sql-script.inc');

// Simplistic message passing for webrtc signaling.  Clients poll the server for
// messages (ajax/action.message.send.inc and ajax/action.message.retrieve.inc).
//
// Each message is stored as a json string.  The 'recipient' field, if there is
// one, is recognized specially by the message.send action.  message.retrieve
// takes a 'recipient' argument and only messages addressed to that recipient
// are returned.
//
// 'camera-replay' is the identifier for the (presumed one and only) remote
// camera.  Each replay kiosk adopts a name with the 'viewer-' prefix for webrtc
// interactions with the remote camera, if it uses one.
//
// In practice, each webrtc message has a 'type' field ('solicitation', 'offer',
// 'ice-candidate', and 'answer') and a 'from' field.
//
// If websocket support is available, the javascript MessagePoller will prefer
// to pass messages via a websocket server and skip using this database- and
// polling-based mechanism.


function create_message_queue_table() {
  run_sql_script('message-queue-table');
}

function message_queue_sql($sql, $params) {
  global $db;

  do {
    $retry = false;
    try {
      $stmt = $db->prepare($sql);
      $stmt->execute($params);
    } catch (PDOException $p) {
      if (!$retry && is_no_such_table_exception($p)) {
        $retry = true;
        create_message_queue_table();
      } else {
        echo "\n<errorInfo>\n"; print_r($db->errorInfo()); echo "\n</errorInfo>";
       
        echo "\n<PDOException code='".$p->getCode()."'>\n";
        echo $p;
        echo "\n</PDOException>\n";
      }
    }
  } while ($retry);
}

// Takes $recipient and $message as strings, $recipient possibly empty.
function send_message($recipient, $message) {
  global $db;
  // A message with a recipient gets deleted when retrieved.  A broadcast has to
  // be removed by timeout, so shouldn't linger too long.
  $timeout = empty($recipient) ? 1 : 15;
  message_queue_sql('INSERT INTO MessageQueue(recipient, expiration, message)'
                    .' VALUES(:recipient, :expiration, :message)',
                    array(':recipient' => $recipient,
                          ':expiration' => time() + $timeout,
                          ':recipient' => $recipient,
                          ':message' => $message));
}

// Returns an array of message strings
function retrieve_messages($recipient) {
  global $db;
  try {
    $stmt = $db->prepare('SELECT msgid, recipient, message FROM MessageQueue'
                         .' WHERE recipient = :recipient OR recipient = \'\''
                         .' ORDER BY expiration');
    $stmt->execute(array(':recipient' => $recipient));
  } catch (PDOException $p) {
    // Failure = no messages
    return array();
  }
  $values = array();
  $to_delete = '';
  foreach ($stmt as $row) {
    if (!empty($row['recipient'])) {
      // An empty recipient is a broadcast, so other parties may want it.
      $to_delete .= ', '.$row['msgid'];
    }
    $values[] = $row['message'];
  }
  if (!empty($to_delete)) {
    $db->exec('DELETE FROM MessageQueue WHERE msgid IN (0'.$to_delete.')');
  }

  return $values;
}

function purge_stale_messages() {
  message_queue_sql('DELETE FROM MessageQueue'
                    .' WHERE expiration < :now',
                    array(':now' => time()));
}

?>
