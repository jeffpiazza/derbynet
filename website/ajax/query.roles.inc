<roles>
<?php
foreach ($roles as $name => $details) {
  echo "<role>".htmlspecialchars($name, ENT_QUOTES, 'UTF-8')."</role>\n";
}
?>
</roles>