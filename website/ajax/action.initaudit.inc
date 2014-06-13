<?php
// TODO: This is Access-only
  take_action('CREATE TABLE CheckinAudit ('
			  .'   seq    COUNTER,'
			  .'   stmt   VARCHAR (250),'
			  .'   tstamp DATETIME'
			  .')',
			  array());
?>