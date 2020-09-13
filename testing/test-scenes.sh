#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

# For kiosk polling, normally the server fills in an address based on the
# client's IP and port.  For testing, we provide explicit query parameter.  The
# address can basically be any string; doesn't have to be an IP address.

# mktemp manufactures a file; we just want the name
KIOSK1=`mktemp kiosk1-XXXXXX`
KIOSK2=`mktemp kiosk2-XXXXXX`
KIOSK3=`mktemp kiosk3-XXXXXX`

rm $KIOSK1 $KIOSK2 $KIOSK3

# Introduce two kiosks; they get identify.kiosk by default
curl_get "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one kiosks/identify.kiosk
curl_get "action.php?query=poll.kiosk&address=$KIOSK2" | expect_one kiosks/identify.kiosk

curl_get "action.php?query=poll.kiosk.all" | expect_one "<address>$KIOSK1</address>"
curl_get "action.php?query=poll.kiosk.all" | expect_one "<address>$KIOSK2</address>"

# Explicitly assign one kiosk page
curl_post action.php "action=kiosk.assign&address=$KIOSK2&page=kiosks/welcome.kiosk" | check_success
curl_get "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one kiosks/identify.kiosk
curl_get "action.php?query=poll.kiosk&address=$KIOSK2" | expect_one kiosks/welcome.kiosk

# Setting a scene has no effect if none of the kiosks are named
curl_post action.php "action=scene.apply&sceneid=1" | check_success
curl_get "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one kiosks/identify.kiosk
curl_get "action.php?query=poll.kiosk&address=$KIOSK2" | expect_one kiosks/welcome.kiosk

# Name the two kiosks.  Assigned pages don't change.
curl_post action.php "action=kiosk.assign&address=$KIOSK1&name=Main" | check_success
curl_post action.php "action=kiosk.assign&address=$KIOSK2&name=Aux1" | check_success
curl_get "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one kiosks/identify.kiosk
curl_get "action.php?query=poll.kiosk&address=$KIOSK2" | expect_one kiosks/welcome.kiosk

# Setting a scene now affects the named kiosks
curl_post action.php "action=scene.apply&sceneid=4" | check_success
curl_get "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one kiosks/now-racing.kiosk
curl_get "action.php?query=poll.kiosk&address=$KIOSK2" | expect_one kiosks/ondeck.kiosk

# Now the third kiosk joins
curl_get "action.php?query=poll.kiosk&address=$KIOSK3" | expect_one kiosks/identify.kiosk
curl_post action.php "action=kiosk.assign&address=$KIOSK3&name=Aux2" | check_success
curl_get "action.php?query=poll.kiosk&address=$KIOSK3" | expect_one kiosks/identify.kiosk

# Re-setting the scene affects the late kiosk
curl_post action.php "action=scene.apply&sceneid=4" | check_success
curl_get "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one kiosks/now-racing.kiosk
curl_get "action.php?query=poll.kiosk&address=$KIOSK2" | expect_one kiosks/ondeck.kiosk
curl_get "action.php?query=poll.kiosk&address=$KIOSK3" | expect_one kiosks/results-by-racer.kiosk

# Switching to a scene without assignments for some kiosks leaves those kiosks
# unaffected
curl_post action.php "action=scene.apply&sceneid=5" | check_success
curl_get "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one kiosks/award-presentations.kiosk
curl_get "action.php?query=poll.kiosk&address=$KIOSK2" | expect_one kiosks/ondeck.kiosk
curl_get "action.php?query=poll.kiosk&address=$KIOSK3" | expect_one kiosks/results-by-racer.kiosk

curl_post action.php "action=scene.apply&sceneid=3" | check_success
curl_get "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one kiosks/slideshow.kiosk
curl_get "action.php?query=poll.kiosk&address=$KIOSK2" | expect_one kiosks/please-check-in.kiosk
curl_get "action.php?query=poll.kiosk&address=$KIOSK3" | expect_one kiosks/welcome.kiosk

curl_post action.php "action=scene.apply&sceneid=6" | check_success
curl_get "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one kiosks/standings.kiosk
curl_get "action.php?query=poll.kiosk&address=$KIOSK2" | expect_one kiosks/derbynet.kiosk
curl_get "action.php?query=poll.kiosk&address=$KIOSK3" | expect_one local/kiosks/feedback.kiosk
