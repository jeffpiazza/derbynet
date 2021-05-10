
<coordinator_poll>

<?php
require('ajax/query.json.poll.coordinator.inc');
echo htmlspecialchars(json_encode($json_out, JSON_PRETTY_PRINT), ENT_QUOTES, 'UTF-8');
?>

</coordinator_poll>
