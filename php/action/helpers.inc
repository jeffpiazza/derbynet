<?php
require_once('data.inc');
require_once('permissions.inc');
require_once('authorize.inc');


function xml_for_reload() {
  return '<reload>1</reload>';
}

function start_response() {
	echo '<action-response';
	foreach ($_POST as $attr => $val) {
		echo ' '.$attr.'="'.$val.'"';
	}
	echo '>';
}

function end_response() {
	echo '</action-response>'."\n";
}

function take_action_silently($sql, $params) {
  global $db;
  $log_sql = $sql;
  foreach ($params as $key => $val) {
	  $log_sql .= ' // '.$key.' => '.$val;
  }
  if (strlen($log_sql) > 250) {
	$log_sql = substr($log_sql, 1, 120) . '...' . substr($log_sql, strlen($log_sql) - 120);
  }

  $stmt = $db->prepare('INSERT INTO CheckinAudit(tstamp, stmt) VALUES(NOW(), :logsql)');
  @$stmt->execute(array(':logsql' => $log_sql));

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

function not_authorized() {
	start_response();
	echo '<failure code="notauthorized">Not authorized -- please see race coordinator</failure>';
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