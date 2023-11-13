<?php

require_once('inc/data.inc');

// TODO Note that coordinator.js also receives these values, but doesn't seem to do anything with them

define('TIMER_NOT_CONNECTED', 1);  // No recent contact from an authenticated timer
define('TIMER_SEARCHING', 6);  // derby-timer.jar made contact, but timer hasn't been identified
define('TIMER_UNCONFIRMED', 7);  // Timer is identified only by the user's claim, and can only be
                                 // presumed healthy, it's never been heard from.
define('TIMER_CONNECTED', 2);      // Timer connected, but not doing anything right now
define('TIMER_STAGING', 3);       // A HEAT-READY message sent to connected timer
define('TIMER_RUNNING', 4);        // A STARTED message received (but not yet a FINISHED message)

define('TIMER_UNHEALTHY', 5);     // A persistently-detectable malfunction;
                                  // will clear itself (good heartbeat) when
                                  // the problem is corrected.

// Note 50-character limit for ItemValue in GPRM schema, but who would be using
// GPRM database directly if they're maintaining DerbyNet timer state?  A
// truncated message is relatively harmless here anyway.  A time() value takes
// 10 digits.

class TimerState {
  // One of the values above.  If not present, no timer contact has occurred,
  // and the state is NOT_CONNECTED.
  private $state;
  
  // The time() value when the derby-timer.jar last contacted the server;
  // initially 0.  If set to a value less than zero (for testing), then the
  // check for recency of contact is disabled.
  private $last_contact;

  // malfunction_expiration='' for "no malfunction",
  // malfunction_expiration=0 for explicit reset required, or
  // malfunction_expiration holds a time() value when the malfunction will expire
  private $malfunction_expiration;
  // A malfunction message if malfunction_expiration != '', or the error message
  // if state == UNHEALTHY
  private $malfunction_message;

  // "Creating" a TimerState actually reads the state out of the database.  Don't forget to do a write()
  // if the state gets updated!
  public function __construct() {
    $this->state = TIMER_NOT_CONNECTED;
    $this->last_contact = 0;
    $this->malfunction_expiration = '';
    $this->malfunction_message = '';

    $raw = read_raceinfo('timer_state', false);
    if ($raw !== false && $raw != '') {
      $cooked = explode('+', $raw, 4);

      // This covers the rare/weird case where there's some leftover value for this
      // key still hanging around in an old database
      if (count($cooked) < 4) {
        if (count($cooked) < 3) {
          if (count($cooked) < 2) {
            $cooked[] = 0;  // last contact
          }
          $cooked[] = '';  // malfunction expiration
        }
        $cooked[] = '';  // malfunction message
      }

      $this->state = $cooked[0];
      $this->last_contact = $cooked[1];
      $this->malfunction_expiration = $cooked[2];
      $this->malfunction_message = $cooked[3];
    }

    if ($this->last_contact >= 0 && $this->last_contact + 60 < time()) {
      $timed_out = ($this->state != TIMER_NOT_CONNECTED);
      $this->state = TIMER_NOT_CONNECTED;
      if ($timed_out) {
        $this->write(false);
        record_action(array('action' => 'timer.*overdue'));
      }
    }
    if ($this->malfunction_expiration != '' &&
        $this->malfunction_expiration != 0 &&
        $this->malfunction_expiration <= time()) {
      $this->malfunction_expiration = '';
      $this->malfunction_message = '';
    }
  }

  // Returns a numeric state value, e.g., TIMER_CONNECTED
  public function state() {
    return $this->state;
  }

  public function last_contact() {
    return $this->last_contact;
  }

  public function seconds_since_last_contact() {
    if ($this->last_contact < 0) {
      return 0;
    } else {
      return time() - $this->last_contact;
    }
  }
  
  // "Unhealthy" implies it might get better on its own, i.e., by getting a good
  // heartbeat from the timer.
  public function unhealthy() {
    return $this->state == TIMER_NOT_CONNECTED ||
           $this->state == TIMER_SEARCHING ||
           $this->state == TIMER_UNHEALTHY;
  }

  // "Troubled" means unhealthy OR a malfunction.
  public function troubled() {
    // malfunction_expiration got cleared in constructor if it expired
    return $this->unhealthy() || $this->malfunction_expiration != '';
  }

  // Returns a string which will be non-empty if a "malfunction" or "unhealthy"
  // condition is in effect.
  public function malfunction() {
    return $this->malfunction_message;
  }

  public function set_malfunction($duration_secs, $message) {
    $this->malfunction_expiration = time() + $duration_secs;
    $this->malfunction_message = $message;
  }

  public function set_unhealthy($message) {
    $this->state = TIMER_UNHEALTHY;
    $this->malfunction_message = $message;
  }

  public function to_state($state) {
    $this->state = $state;
  }

  public function set_assume_connected_for_test() {
    $this->last_contact = -1;
    if ($this->unhealthy()) {
      $this->to_state(TIMER_CONNECTED);
    }
  }

  public function write($renew) {
    if ($renew && $this->last_contact >= 0) {
      $this->last_contact = time();
    }
    write_raceinfo('timer_state', implode('+', array($this->state, $this->last_contact,
                                                     $this->malfunction_expiration,
                                                     $this->malfunction_message)));
  }
}
?>
