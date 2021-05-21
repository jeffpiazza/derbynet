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
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK1" | expect_one identify.kiosk
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK2" | expect_one identify.kiosk

curl_getj "action.php?query=json.poll.kiosk.all" | expect_one "\"address\": \"$KIOSK1\""
curl_getj "action.php?query=json.poll.kiosk.all" | expect_one "\"address\": \"$KIOSK2\""

# Explicitly assign one kiosk page
curl_postj action.php "action=json.kiosk.assign&address=$KIOSK2&page=kiosks/welcome.kiosk" | check_jsuccess
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK1" | expect_one identify.kiosk
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK2" | expect_one welcome.kiosk

# Setting a scene has no effect if none of the kiosks are named
curl_postj action.php "action=json.scene.apply&sceneid=1" | check_jsuccess
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK1" | expect_one identify.kiosk
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK2" | expect_one welcome.kiosk

# Name the two kiosks.  Assigned pages change to match the current scene.
curl_postj action.php "action=json.kiosk.assign&address=$KIOSK1&name=Main" | check_jsuccess
curl_postj action.php "action=json.kiosk.assign&address=$KIOSK2&name=Aux1" | check_jsuccess
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK1" | expect_one welcome.kiosk
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK2" | expect_one welcome.kiosk

# Setting a scene now affects the named kiosks
curl_postj action.php "action=json.scene.apply&sceneid=4" | check_jsuccess
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK1" | expect_one now-racing.kiosk
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK2" | expect_one ondeck.kiosk

# Now the third kiosk joins
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK3" | expect_one identify.kiosk
curl_postj action.php "action=json.kiosk.assign&address=$KIOSK3&name=Aux2" | check_jsuccess
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK3" | expect_one results-by-racer.kiosk

# Re-setting the scene affects the late kiosk
curl_postj action.php "action=json.scene.apply&sceneid=4" | check_jsuccess
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK1" | expect_one now-racing.kiosk
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK2" | expect_one ondeck.kiosk
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK3" | expect_one results-by-racer.kiosk

# Switching to a scene without assignments for some kiosks leaves those kiosks
# unaffected
curl_postj action.php "action=json.scene.apply&sceneid=5" | check_jsuccess
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK1" | expect_one award-presentations.kiosk
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK2" | expect_one ondeck.kiosk
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK3" | expect_one results-by-racer.kiosk

curl_postj action.php "action=json.scene.apply&sceneid=3" | check_jsuccess
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK1" | expect_one slideshow.kiosk
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK2" | expect_one please-check-in.kiosk
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK3" | expect_one welcome.kiosk

curl_postj action.php "action=json.scene.apply&sceneid=6" | check_jsuccess
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK1" | expect_one standings.kiosk
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK2" | expect_one derbynet.kiosk
curl_getj "action.php?query=json.poll.kiosk&address=$KIOSK3" | expect_one feedback.kiosk
