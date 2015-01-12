#! /bin/sh

BASE_URL=$1
source `dirname $0`/common.sh

PATTERN='Notice\|Warning\|Fatal'
curl_get about.php | grep $PATTERN
curl_get awards.php | grep $PATTERN
curl_get awards-dashboard.php | grep $PATTERN
curl_get checkin.php | grep $PATTERN
curl_get coordinator.php | grep $PATTERN
curl_get import-roster.php | grep $PATTERN
curl_get index.php | grep $PATTERN
curl_get judging.php | grep $PATTERN
curl_get login.php | grep $PATTERN
curl_get ondeck.php | grep $PATTERN
curl_get photo-thumbs.php | grep $PATTERN
# curl_get photo-crop.php | grep $PATTERN
curl_get racer-results.php | grep $PATTERN
curl_get settings.php | grep $PATTERN
curl_get setup.php | grep $PATTERN

curl_get "kiosk.php?page=kiosks/identify.kiosk" | grep $PATTERN
curl_get "kiosk.php?page=kiosks/welcome.kiosk" | grep $PATTERN
curl_get "kiosk.php?page=kiosks/please_check_in.kiosk" | grep $PATTERN
curl_get "kiosk.php?page=kiosks/ondeck.kiosk" | grep $PATTERN
curl_get "kiosk.php?page=kiosks/now-racing.kiosk" | grep $PATTERN
curl_get "kiosk.php?page=kiosks/award-presentations.kiosk" | grep $PATTERN
