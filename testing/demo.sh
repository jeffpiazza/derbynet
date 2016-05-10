#! /bin/bash

# Runs an unattended demo of the software.  Stand up a web server somewhere, and
# a kiosk that points to it.  Run this script, pointed at the server.

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

`dirname $0`/login-coordinator.sh $BASE_URL

while true ; do
  curl_post action.php "action=run-sql&script=schema" | check_success
  curl_post action.php "action=run-sql&script=update-schema" | check_success
  curl_post action.php "action=write-settings&show-racer-photos=1&show-racer-photos-checkbox=1" | check_success
  curl_post action.php "action=write-settings&photo-width=180&photo-height=240" | check_success

  # kiosk page will re-poll every 5s
  sleep 6s
  ########## Please Check In ##############
  curl_post action.php "action=assign-kiosk&all=kiosks/please-check-in.kiosk" | check_success

  `dirname $0`/import-roster.sh $BASE_URL
  `dirname $0`/test-photo-assignments.sh $BASE_URL
  sleep 6s

  ########## Slideshow ##############
  curl_post action.php "action=assign-kiosk&all=kiosks/slideshow.kiosk" | check_success

  curl_post action.php "action=write-settings&n-lanes=4" | check_success

  `dirname $0`/checkin-all.sh "$BASE_URL"
  sleep 15s

  curl_post action.php "action=class.edit&classid=1&name=Tigers" | check_success
  curl_post action.php "action=class.edit&classid=2&name=Wolves" | check_success
  curl_post action.php "action=class.edit&classid=3&name=Bears" | check_success
  curl_post action.php "action=class.edit&classid=4&name=Webelos%20I" | check_success
  curl_post action.php "action=class.edit&classid=5&name=Webelos%20II" | check_success

  curl_post action.php "action=class.order&classid_1=1&classid_2=2&classid_3=3&classid_4=4&classid_5=5" | check_success

  # Start the timer
  curl_post action.php "action=timer-message&message=HELLO" | check_success
  curl_post action.php "action=timer-message&message=IDENTIFIED&lane_count=4" | check_success

  # Schedule Round 1
  curl_post action.php "action=schedule&roundid=1" | check_success
  curl_post action.php "action=schedule&roundid=2" | check_success
  # curl_post action.php "action=schedule&roundid=3" | check_success
  # curl_post action.php "action=schedule&roundid=4" | check_success
  # curl_post action.php "action=schedule&roundid=5" | check_success

  ########## On Deck ##############
  curl_post action.php "action=assign-kiosk&all=kiosks/ondeck.kiosk" | check_success

  # Start racing
  curl_post action.php "action=select-heat&now_racing=1&roundid=1" | check_success

  sleep 1s
  curl_post action.php "action=timer-message&message=STARTED" | check_success
  sleep 2s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=3.2514&lane2=3.9601&lane3=2.1068&lane4=2.4405" | check_success
  sleep 3s
  curl_post action.php "action=timer-message&message=STARTED" | check_success
  sleep 2s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=3.5577&lane2=2.2320&lane3=3.8407&lane4=3.2743" | check_success
  sleep 3s
  curl_post action.php "action=timer-message&message=STARTED" | check_success
  sleep 2s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=3.0326&lane2=2.1680&lane3=2.8041&lane4=2.7058" | check_success
  sleep 3s
  curl_post action.php "action=timer-message&message=STARTED" | check_success
  sleep 2s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=3.5629&lane2=2.3208&lane3=2.7011&lane4=2.9505" | check_success
  sleep 3s
  curl_post action.php "action=timer-message&message=STARTED" | check_success
  sleep 2s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=3.8318&lane2=2.6336&lane3=3.7829&lane4=3.8474" | check_success
  sleep 3s

  ########## Now-Racing ##############
  curl_post action.php "action=assign-kiosk&all=kiosks/now-racing.kiosk" | check_success

  curl_post action.php "action=timer-message&message=STARTED" | check_success
  sleep 4s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=2.8646&lane2=3.5764&lane3=3.8810&lane4=2.2240" | check_success
  sleep 15s
  curl_post action.php "action=timer-message&message=STARTED" | check_success
  sleep 4s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=2.4750&lane2=2.4742&lane3=2.8326&lane4=3.2385" | check_success
  sleep 15s
  curl_post action.php "action=timer-message&message=STARTED" | check_success
  sleep 4s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=3.3017&lane2=3.9341&lane3=3.0639&lane4=3.0011" | check_success
  sleep 15s
  curl_post action.php "action=timer-message&message=STARTED" | check_success
  sleep 4s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=3.2278&lane2=3.1430&lane3=2.3543&lane4=3.0481" | check_success
  sleep 15s
  curl_post action.php "action=timer-message&message=STARTED" | check_success
  sleep 4s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=3.8930&lane2=3.9921&lane3=3.8794&lane4=2.8338" | check_success
  sleep 15s

  ########## Results-By-Racer ##############
  curl_post action.php "action=assign-kiosk&all=kiosks/results-by-racer.kiosk" | check_success

  curl_post action.php "action=timer-message&message=STARTED" | check_success
  sleep 2s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=2.7335&lane2=2.4780&lane3=3.5128&lane4=2.3030" | check_success
  sleep 3s
  curl_post action.php "action=timer-message&message=STARTED" | check_success
  sleep 2s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=2.5309&lane2=3.6360&lane3=3.0660&lane4=2.8207" | check_success
  sleep 3s
  curl_post action.php "action=timer-message&message=STARTED" | check_success
  sleep 2s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=3.7994&lane2=3.3811&lane3=2.6539&lane4=3.2623" | check_success
  sleep 3s
  curl_post action.php "action=timer-message&message=STARTED" | check_success
  sleep 2s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=3.5108&lane2=3.0641&lane3=3.6981&lane4=2.9022" | check_success
  sleep 3s
  curl_post action.php "action=timer-message&message=STARTED" | check_success
  sleep 2s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=2.6329&lane2=2.2925&lane3=2.2111&lane4=3.8931" | check_success
  sleep 3s
  curl_post action.php "action=timer-message&message=STARTED" | check_success
  sleep 2s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=2.7492&lane2=2.6261&lane3=3.9556&lane4=3.5883" | check_success
  sleep 3s
  curl_post action.php "action=timer-message&message=STARTED" | check_success
  sleep 2s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=2.6295&lane2=3.1224&lane3=2.8310&lane4=2.7356" | check_success
  sleep 3s

  ########## Awards Presentations ##############
  curl_post action.php "action=assign-kiosk&all=kiosks/award-presentations.kiosk" | check_success
  sleep 6s
  curl_post action.php "action=award.present&key=speed-1-1" | check_success
  sleep 15s

  ########## DerbyNet ##############
  curl_post action.php "action=assign-kiosk&all=kiosks/derbynet.kiosk" | check_success
  sleep 30s
done
