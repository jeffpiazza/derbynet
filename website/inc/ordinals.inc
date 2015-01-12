<?php

function ordinal($n) {
  if ($n > 20 && $n % 10 != 0) {
    return floor($n / 10).ordinal($n % 10);
  } else if ($n < 4) {
    $a = array(1 => '1st', 2 => '2nd', 3 => '3rd');
	return $a[$n];
  } else {
    return $n.'th';
  }
}

?>