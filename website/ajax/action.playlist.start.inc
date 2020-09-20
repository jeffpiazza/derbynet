<?php

require_once('inc/playlist.inc');
require_once('inc/racing-state.inc');


start_response();

activate_playlist_entry(first_playlist_entry());
set_racing_state(1);

echo "<success/>";

end_response();

?>