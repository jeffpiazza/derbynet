<?php
session_start();

$simulated_polling_url = 'js/simulated-polling.js';

function get_lane_count() { return 4; }

require_once('inc/kiosks.inc');
$_GET['page'] = 'kiosks/now-racing.kiosk';
require('kiosks/now-racing.kiosk');
?>
