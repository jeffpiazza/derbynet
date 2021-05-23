<?php

require_once('inc/voterid.inc');
json_out('awards', vote_tabulations());

?>