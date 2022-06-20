#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

PATTERN='Notice\|Warning\|Fatal'
curl_get about.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get awards-editor.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get awards-presentation.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get camera.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get checkin.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get coordinator.php | sed -n -e "/$PATTERN/ { p; q 1 }"
# curl_get export-json.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get export.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get fake-timer.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get fakeroster.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get fullscreen.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get history.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get import-awards.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get import-results.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get import-roster.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get import-snapshot.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get index.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get judging.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get kiosk-dashboard.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get login.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get ondeck.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get photo-thumbs.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get playlist.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get print.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get racer-results.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get racing-groups.php | sed -n -e "/$PATTERN/ { p; q 1 }"
# curl_get replay.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get scenes.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get settings.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get setup.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get standings.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get timer.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get timer-test.php | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get vote.php | sed -n -e "/$PATTERN/ { p; q 1 }"

curl_get "kiosk.php?page=kiosks/award-presentations.kiosk" | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get "kiosk.php?page=kiosks/identify.kiosk" | sed -n -e "/$PATTERN/ { p; q 1 }" 
curl_get "kiosk.php?page=kiosks/now-racing.kiosk" | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get "kiosk.php?page=kiosks/ondeck.kiosk" | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get "kiosk.php?page=kiosks/please-check-in.kiosk" | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get "kiosk.php?page=kiosks/results-by-racer.kiosk" | sed -n -e "/$PATTERN/ { p; q 1 }"
curl_get "kiosk.php?page=kiosks/welcome.kiosk" | sed -n -e "/$PATTERN/ { p; q 1 }"

# Should do a more complete test of the interaction between replay and the
# server.  This just ensures that the basic replay.message code hasn't broken
# completely.
curl_postj action.php "action=replay.message&status=-1&finished-replay=0" | check_jsuccess
