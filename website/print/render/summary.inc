<?php

require_once('inc/data.inc');
require_once('inc/schema_version.inc');
require_once('print/inc/printable_summary_document.inc');
require_once('fpdf/ext/combined.php');

$doc = new_document();

$doc->StartDocument();
$doc->Draw();
$doc->Output();

?>