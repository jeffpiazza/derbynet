#! /bin/sh

BASE_URL=$1
source common.sh

curl_get about.php > /dev/null
curl_get awards.php > /dev/null
curl_get checkin.php > /dev/null
curl_get coordinator.php > /dev/null
curl_get import-roster.php > /dev/null
curl_get index.php > /dev/null
curl_get judging.php > /dev/null
curl_get login.php > /dev/null
curl_get ondeck.php > /dev/null
curl_get photo-thumbs.php > /dev/null
# curl_get photo-crop.php > /dev/null
curl_get racer-results.php > /dev/null
curl_get settings.php > /dev/null
curl_get setup.php > /dev/null

curl_get "kiosk.php?page=kiosks/identify.kiosk" > /dev/null
curl_get "kiosk.php?page=kiosks/now-racing.kiosk" > /dev/null
curl_get "kiosk.php?page=kiosks/ondeck.kiosk" > /dev/null
curl_get "kiosk.php?page=kiosks/please_check_in.kiosk" > /dev/null
curl_get "kiosk.php?page=kiosks/welcome.kiosk" > /dev/null
