<?php
  require_once('inc/awards-xml.inc');

  // $_GET['adhoc'] = 0/1 to exclude/include ad hoc awards
  emit_award_list(isset($_GET['adhoc']) ? $_GET['adhoc'] : 1);
?>
