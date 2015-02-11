#! /bin/sh

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

PATTERN='Notice\|Warning\|Fatal'
curl_get about.php | test ! `grep $PATTERN`
curl_get awards.php | test ! `grep $PATTERN`
curl_get awards-dashboard.php | test ! `grep $PATTERN`
curl_get checkin.php | test ! `grep $PATTERN`
curl_get coordinator.php | test ! `grep $PATTERN`
curl_get import-roster.php | test ! `grep $PATTERN`
curl_get index.php | test ! `grep $PATTERN`
curl_get judging.php | test ! `grep $PATTERN`
curl_get login.php | test ! `grep $PATTERN`
curl_get ondeck.php | test ! `grep $PATTERN`
curl_get photo-thumbs.php | test ! `grep $PATTERN` || true
# curl_get photo-crop.php | test ! `grep $PATTERN`
curl_get racer-results.php | test ! `grep $PATTERN`
curl_get settings.php | test ! `grep $PATTERN`
curl_get setup.php | test ! `grep $PATTERN`

curl_get "kiosk.php?page=kiosks/award-presentations.kiosk" | test ! `grep $PATTERN`
curl_get "kiosk.php?page=kiosks/identify.kiosk" | test ! `grep $PATTERN`
curl_get "kiosk.php?page=kiosks/now-racing.kiosk" | test ! `grep $PATTERN`
curl_get "kiosk.php?page=kiosks/ondeck.kiosk" | test ! `grep $PATTERN`
curl_get "kiosk.php?page=kiosks/please-check-in.kiosk" | test ! `grep $PATTERN`
curl_get "kiosk.php?page=kiosks/results-by-racer.kiosk" | test ! `grep $PATTERN`
curl_get "kiosk.php?page=kiosks/welcome.kiosk" | test ! `grep $PATTERN`
