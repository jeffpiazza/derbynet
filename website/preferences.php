<?php @session_start();

// In contrast to query.snapshot.get.inc, this download link stands on its own
// in order to have full control over the Content-Type headers, without
// interference from action.php.

require_once('inc/preferences.inc');
session_write_close();

header('Content-Type: text/plain');
// header('Content-Disposition: attachment; '.$this->_httpencode('filename',$name,$isUTF8));
header('Cache-Control: private, max-age=0, must-revalidate');
header('Pragma: public');

echo dump_preferences();
?>
