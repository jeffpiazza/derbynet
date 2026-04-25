<?php

require_once('inc/data.inc');

function salt_for_token() {
  $salt = read_raceinfo('token-salt');
  if ($salt === false) {
    // Random two-character salt
    $chars = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()';
    $salt = $chars[random_int(0, strlen($chars) - 1)].$chars[random_int(0, strlen($chars) - 1)];
    write_raceinfo('token-salt', $salt);
  }
  return $salt;
}

// Real crypto would produce a longer string than we want
function hash_for_mcheckin_expiration($expiration) {
  return substr(hash_hmac('md5', $expiration, salt_for_token()), 3, 6);
}

function token_for_mcheckin($duration_mins = 60) {
  $expiration = intdiv(time(), 60) + $duration_mins;
  // return $expiration."/".bin2hex(crypt($expiration, salt_for_token()));
  return $expiration.".".hash_for_mcheckin_expiration($expiration);
}

function validate_token_for_mcheckin($tok) {
  $tok = explode('.', $tok, 2);
  if (count($tok) < 2) return false;  // ill-formed
  $expiration = $tok[0];
  if ($expiration <= intdiv(time(), 60)) return false;  // expired
  return $tok[1] == hash_for_mcheckin_expiration($expiration);
}
?>
