#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

echo demo-multi assumes you have set up the following:
echo Main "(laptop)" kiosk 1920x1080 id=main
echo Auxiliary "(rpi3)" kiosk id=aux
echo Wrangler tablet kiosk id=ondeck
echo Race Coordinator page on a tablet/iPad
# TODO MacReplay demo?

MAIN=main
AUX=aux
ONDECK=ondeck

user_login_coordinator

while true ; do
  # Resetting the database also clears all the kiosk settings
  curl_post action.php "action=database.execute&script=schema" | check_success
  curl_post action.php "action=database.execute&script=update-schema" | check_success

  # Because the kiosks haven't registered themselves (again) since the database
  # got reset.  We don't want to use the cookies file because we don't want the
  # same session.
  curl --location -s "$BASE_URL/kiosk.php?id=$MAIN" > /dev/null
  curl --location -s "$BASE_URL/kiosk.php?id=$AUX" > /dev/null

  curl_post action.php "action=kiosk.assign&address=$MAIN&page=kiosks/slideshow.kiosk" | check_success
  curl_post action.php "action=kiosk.assign&address=$AUX&page=kiosks/welcome.kiosk" | check_success

  # Tedious set-up
  curl_post action.php "action=settings.write&photos-on-now-racing=head" | check_success
  curl_post action.php "action=settings.write&show-car-photos-on-deck=1&show-car-photos-on-deck-checkbox=1" | check_success

  `dirname $0`/import-roster.sh "$BASE_URL"
  `dirname $0`/photo-setup.sh "$BASE_URL"
  curl_post action.php "action=class.edit&classid=1&name=Tigers" | check_success
  curl_post action.php "action=class.edit&classid=2&name=Wolves" | check_success
  curl_post action.php "action=class.edit&classid=3&name=Bears" | check_success
  curl_post action.php "action=class.edit&classid=4&name=Webelos%20I" | check_success
  curl_post action.php "action=class.edit&classid=5&name=Webelos%20II" | check_success
  curl_post action.php "action=class.order&classid_1=1&classid_2=2&classid_3=3&classid_4=4&classid_5=5" | check_success
  `dirname $0`/test-photo-assignments.sh "$BASE_URL"
  curl_post action.php "action=settings.write&n-lanes=4" | check_success

  curl_post action.php "action=kiosk.assign&address=$AUX&page=kiosks/please-check-in.kiosk" | check_success

  `dirname $0`/checkin-all.sh "$BASE_URL"

  sleep 30s
  user_login_timer
  # Start the timer
  curl_post action.php "action=timer-message&message=HELLO" | check_success
  curl_post action.php "action=timer-message&message=IDENTIFIED&lane_count=4" | check_success
  user_login_coordinator

  # Schedule Round 1
  curl_postj action.php "action=json.schedule.generate&roundid=1" | check_jsuccess
  # curl_postj action.php "action=json.schedule.generate&roundid=2" | check_jsuccess
  # curl_postj action.php "action=json.schedule.generate&roundid=3" | check_jsuccess
  # curl_postj action.php "action=json.schedule.generate&roundid=4" | check_jsuccess
  # curl_postj action.php "action=json.schedule.generate&roundid=5" | check_jsuccess

  # Start racing
  curl_post action.php "action=select-heat&now_racing=1&roundid=1" | check_success

  curl_post action.php "action=settings.write&show-car-photos-on-deck=1&show-car-photos-on-deck-checkbox=1" | check_success
  curl_post action.php "action=kiosk.assign&address=$ONDECK&page=kiosks/ondeck.kiosk" | check_success
  curl_post action.php "action=kiosk.assign&address=$MAIN&page=kiosks/now-racing.kiosk" | check_success
  curl_post action.php "action=kiosk.assign&address=$AUX&page=kiosks/results-by-racer.kiosk" | check_success

  # Knock off a few early rounds
  user_login_timer
  curl_post action.php "action=timer-message&message=FINISHED&lane1=3.2514&lane2=3.9601&lane3=2.1068&lane4=2.4405" | check_success
  curl_post action.php "action=timer-message&message=FINISHED&lane1=3.5577&lane2=2.2320&lane3=3.8407&lane4=3.2743" | check_success
  curl_post action.php "action=timer-message&message=FINISHED&lane1=3.0326&lane2=2.1680&lane3=2.8041&lane4=2.7058" | check_success
  curl_post action.php "action=timer-message&message=FINISHED&lane1=3.5629&lane2=2.3208&lane3=2.7011&lane4=2.9505" | check_success
  curl_post action.php "action=timer-message&message=FINISHED&lane1=3.8318&lane2=2.6336&lane3=3.7829&lane4=3.8474" | check_success
  curl_post action.php "action=timer-message&message=FINISHED&lane1=2.8646&lane2=3.5764&lane3=3.8810&lane4=2.2240" | check_success

  # Full timing for a few heats
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

  # Move through the last heats quickly
  # curl_post action.php "action=timer-message&message=STARTED" | check_success
  # sleep 2s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=2.7335&lane2=2.4780&lane3=3.5128&lane4=2.3030" | check_success
  # sleep 3s
  curl_post action.php "action=timer-message&message=STARTED" | check_success
  # sleep 2s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=2.5309&lane2=3.6360&lane3=3.0660&lane4=2.8207" | check_success
  # sleep 3s
  curl_post action.php "action=timer-message&message=STARTED" | check_success
  # sleep 2s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=3.7994&lane2=3.3811&lane3=2.6539&lane4=3.2623" | check_success
  # sleep 3s
  curl_post action.php "action=timer-message&message=STARTED" | check_success
  # sleep 2s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=3.5108&lane2=3.0641&lane3=3.6981&lane4=2.9022" | check_success
  # sleep 3s
  curl_post action.php "action=timer-message&message=STARTED" | check_success
  # sleep 2s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=2.6329&lane2=2.2925&lane3=2.2111&lane4=3.8931" | check_success
  # sleep 3s
  curl_post action.php "action=timer-message&message=STARTED" | check_success
  # sleep 2s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=2.7492&lane2=2.6261&lane3=3.9556&lane4=3.5883" | check_success
  # sleep 3s
  curl_post action.php "action=timer-message&message=STARTED" | check_success
  # sleep 2s
  curl_post action.php "action=timer-message&message=FINISHED&lane1=2.6295&lane2=3.1224&lane3=2.8310&lane4=2.7356" | check_success
  # sleep 3s

  # TODO Not ready for prime time
  # curl_post action.php "action=kiosk.assign&address=$MAIN&page=kiosks/standings.kiosk" | check_success
  # curl_post action.php "action=standings.select&roundid=1&expose=all" | check_success

  user_login_coordinator
  ########## Awards Presentations ##############
  sleep 15s
  curl_post action.php "action=kiosk.assign&address=$AUX&page=kiosks/award-presentations.kiosk" | check_success
  sleep 6s
  curl_postj action.php "action=json.award.present&key=speed-2-1&reveal=0" | check_jsuccess
  sleep 3s
  curl_postj action.php "action=json.award.present&reveal=1" | check_jsuccess
  sleep 12s
  curl_postj action.php "action=json.award.present&key=speed-1-1&reveal=0" | check_jsuccess
  sleep 3s
  curl_postj action.php "action=json.award.present&reveal=1" | check_jsuccess
  sleep 12s

  ########## DerbyNet ##############
  curl_post action.php "action=kiosk.assign&all=kiosks/derbynet.kiosk" | check_success
  sleep 30s
done
