<?php

start_response();

$stmt = $db->prepare('UPDATE Kiosks SET page = :page WHERE address = :address');
$stmt->execute(array(':page' => $_POST['page'],
                     ':address' => $_POST['address']));
echo "<success/>\n";

end_response();
?>