<?php

require_once('inc/schema_version.inc');
require_once('inc/sql-script.inc');


// Simple message passing for webrtc signaling purposes.  Clients poll the
// server for messages.

function create_message_queue_table() {
  run_sql_script('message-queue-table', true);
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
        echo "<creating-table/>\n";  // TODO
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

function send_message($recipient, $message) {
  global $db;
  message_queue_sql('INSERT INTO MessageQueue(recipient, expiration, message)'
                    .' VALUES(:recipient, :expiration, :message)',
                    array(':recipient' => $recipient,
                          ':expiration' => time() + 30,  // 30-second time to live
                          ':message' => $message));
}

// Returns an array of message strings
function retrieve_messages($recipient) {
  global $db;
  try {
    $stmt = $db->prepare('SELECT msgid, message FROM MessageQueue'
                         .' WHERE recipient = :recipient'
                         .' ORDER BY expiration');
    $stmt->execute(array(':recipient' => $recipient));
  } catch (PDOException $p) {
    // Failure = no messages
    return array();
  }
  $values = array();
  $to_delete = '';
  foreach ($stmt as $row) {
    $to_delete .= ', '.$row['msgid'];
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