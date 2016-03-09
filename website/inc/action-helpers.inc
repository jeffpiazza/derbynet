<?php
require_once('inc/permissions.inc');
require_once('inc/authorize.inc');


function xml_for_reload() {
  return '<reload>1</reload>';
}

function start_response() {
  echo '<action-response';
  foreach ($_POST as $attr => $val) {
    echo ' '.$attr.'="'.htmlspecialchars($val, ENT_QUOTES, 'UTF-8').'"';
  }
  echo '>';
}

function end_response() {
	echo '</action-response>'."\n";
}

function take_action_silently($sql, $params) {
  global $db;
  if (false) {  // Turn this on for debugging actions gone wrong.  Best to make
                // sure there's a CheckinAudit table in that case.
    $log_sql = $sql;
    foreach ($params as $key => $val) {
	  $log_sql .= ' // '.$key.' => '.$val;
    }
    if (strlen($log_sql) > 250) {
      $log_sql = substr($log_sql, 0, 120) . '...' . substr($log_sql, strlen($log_sql) - 120);
    }

    try {
      $stmt = $db->prepare('INSERT INTO CheckinAudit(tstamp, stmt) VALUES('.dbnow_expr().', :logsql)');
      @$stmt->execute(array(':logsql' => $log_sql));
    } catch (PDOException $p) {
      echo '<audit-failed>'.$log_sql.'</audit-failed>'."\n";  // TODO
      // What to do here?  Most likely it's a GPRM database that just doesn't have
      // an audit table.  We could try to create the table and try again, but the
      // risk seems to outweigh the benefit.
      //
      // if ($p->getCode() == '42S02') {
      //   run_sql_script('audit-table');
      // }
    }
  }

  $stmt = $db->prepare($sql);
  return $stmt->execute($params);
}

function take_action($sql, $params, $body_xml = '') {
	global $db;
	start_response();
	if (take_action_silently($sql, $params)) {
		echo'<success/>'.$body_xml;
	} else {
		$info = $db->errorInfo();
		echo '<failure code="sql'.$info[0].'-'.$info[1].'">'
		     .$sql.' failed: '.$info[2].' [EOM]</failure>';
	}
	end_response();
}

function not_authorized_body() {
  echo '<failure code="notauthorized">Not authorized -- please see race coordinator';
  echo '</failure>';
}

function not_authorized() {
	start_response();
	not_authorized_body();
	end_response();
}


function take_action_if_permitted($sql, $params, $permission, $body_xml = '') {
	if (have_permission($permission)) {
		take_action($sql, $params, $body_xml);
	} else {
		not_authorized();
	}
}






?>