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

curl_getj "action.php?query=scene.list" | \
    jq -e '.scenes | length == 6 and
             .[0].name == "Welcome" and
             (.[0].kiosks | length == 1) and
             .[0].kiosks[0].kiosk_name == "Main" and
             .[0].kiosks[0].page == "kiosks\/welcome.kiosk" and
             .[0].kiosks[0].parameters == "{}" and
             .[1].name == "Opening" and
             (.[1].kiosks | length == 3) and
             .[1].kiosks[0].kiosk_name == "Aux1" and
             .[1].kiosks[0].page == "kiosks\/flag.kiosk" and
             .[1].kiosks[0].parameters == "{}" and
             .[1].kiosks[1].kiosk_name == "Aux2" and
             .[1].kiosks[1].page == "kiosks\/flag.kiosk" and
             .[1].kiosks[1].parameters == "{}" and
             .[1].kiosks[2].kiosk_name == "Main" and
             .[1].kiosks[2].page == "kiosks\/flag.kiosk" and
             .[1].kiosks[2].parameters == "{}" and
             .[2].name == "Check-In" and
             (.[2].kiosks | length == 3) and
             .[2].kiosks[0].kiosk_name == "Aux1" and
             .[2].kiosks[0].page == "kiosks\/please-check-in.kiosk" and
             .[2].kiosks[0].parameters == "{}" and
             .[2].kiosks[1].kiosk_name == "Aux2" and
             .[2].kiosks[1].page == "kiosks\/welcome.kiosk" and
             .[2].kiosks[1].parameters == "{}" and
             .[2].kiosks[2].kiosk_name == "Main" and
             .[2].kiosks[2].page == "kiosks\/slideshow.kiosk" and
             .[2].kiosks[2].parameters == "{}" and
             .[3].name == "Racing" and
             (.[3].kiosks | length == 3) and
             .[3].kiosks[0].kiosk_name == "Aux1" and
             .[3].kiosks[0].page == "kiosks\/ondeck.kiosk" and
             .[3].kiosks[0].parameters == "{}" and
             .[3].kiosks[1].kiosk_name == "Aux2" and
             .[3].kiosks[1].page == "kiosks\/results-by-racer.kiosk" and
             .[3].kiosks[1].parameters == "{}" and
             .[3].kiosks[2].kiosk_name == "Main" and
             .[3].kiosks[2].page == "kiosks\/now-racing.kiosk" and
             .[3].kiosks[2].parameters == "{}" and
             .[4].name == "Awards" and
             (.[4].kiosks | length == 1) and
             .[4].kiosks[0].kiosk_name == "Main" and
             .[4].kiosks[0].page == "kiosks\/award-presentations.kiosk" and
             .[4].kiosks[0].parameters == "{}" and
             .[5].name == "Wrap-Up"
' >/dev/null || test_fails

curl_postj action.php "action=scene.setkiosk&sceneid=5&kiosk_name=Main&page=kiosks/award-presentations.kiosk&params=%7Bconfetti%3Afalse%7D" | check_jsuccess

curl_getj "action.php?query=scene.list" | \
    jq -e '.scenes | length == 6 and
             .[4].name == "Awards" and
             (.[4].kiosks | length == 1) and
             .[4].kiosks[0].kiosk_name == "Main" and
             .[4].kiosks[0].page == "kiosks\/award-presentations.kiosk" and
             .[4].kiosks[0].parameters == "{confetti:false}"
' >/dev/null || test_fails

# Undo the params assignment so the test can be re-run if desired.
curl_postj action.php "action=scene.setkiosk&sceneid=5&kiosk_name=Main&page=kiosks/award-presentations.kiosk" | check_jsuccess

# Introduce two kiosks; they get identify.kiosk by default
curl_getj "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one identify.kiosk
curl_getj "action.php?query=poll.kiosk&address=$KIOSK2" | expect_one identify.kiosk

curl_getj "action.php?query=poll.kiosk.all" | expect_one "\"address\": \"$KIOSK1\""
curl_getj "action.php?query=poll.kiosk.all" | expect_one "\"address\": \"$KIOSK2\""

# Explicitly assign one kiosk page
curl_postj action.php "action=kiosk.assign&address=$KIOSK2&page=kiosks/welcome.kiosk" | check_jsuccess
curl_getj "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one identify.kiosk
curl_getj "action.php?query=poll.kiosk&address=$KIOSK2" | expect_one welcome.kiosk

# Setting a scene has no effect if none of the kiosks are named
curl_postj action.php "action=scene.apply&sceneid=1" | check_jsuccess
curl_getj "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one identify.kiosk
curl_getj "action.php?query=poll.kiosk&address=$KIOSK2" | expect_one welcome.kiosk

# Name the two kiosks.  Assigned pages change to match the current scene.
curl_postj action.php "action=kiosk.assign&address=$KIOSK1&name=Main" | check_jsuccess
curl_postj action.php "action=kiosk.assign&address=$KIOSK2&name=Aux1" | check_jsuccess
curl_getj "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one welcome.kiosk
curl_getj "action.php?query=poll.kiosk&address=$KIOSK2" | expect_one welcome.kiosk

# Setting a scene now affects the named kiosks
curl_postj action.php "action=scene.apply&sceneid=4" | check_jsuccess
curl_getj "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one now-racing.kiosk
curl_getj "action.php?query=poll.kiosk&address=$KIOSK2" | expect_one ondeck.kiosk

# Now the third kiosk joins
curl_getj "action.php?query=poll.kiosk&address=$KIOSK3" | expect_one identify.kiosk
curl_postj action.php "action=kiosk.assign&address=$KIOSK3&name=Aux2" | check_jsuccess
curl_getj "action.php?query=poll.kiosk&address=$KIOSK3" | expect_one results-by-racer.kiosk

# Re-setting the scene affects the late kiosk
curl_postj action.php "action=scene.apply&sceneid=4" | check_jsuccess
curl_getj "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one now-racing.kiosk
curl_getj "action.php?query=poll.kiosk&address=$KIOSK2" | expect_one ondeck.kiosk
curl_getj "action.php?query=poll.kiosk&address=$KIOSK3" | expect_one results-by-racer.kiosk

# Switching to a scene without assignments for some kiosks leaves those kiosks
# unaffected
curl_postj action.php "action=scene.apply&sceneid=5" | check_jsuccess
curl_getj "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one award-presentations.kiosk
curl_getj "action.php?query=poll.kiosk&address=$KIOSK2" | expect_one ondeck.kiosk
curl_getj "action.php?query=poll.kiosk&address=$KIOSK3" | expect_one results-by-racer.kiosk

curl_postj action.php "action=scene.apply&sceneid=3" | check_jsuccess
curl_getj "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one slideshow.kiosk
curl_getj "action.php?query=poll.kiosk&address=$KIOSK2" | expect_one please-check-in.kiosk
curl_getj "action.php?query=poll.kiosk&address=$KIOSK3" | expect_one welcome.kiosk

curl_postj action.php "action=scene.apply&sceneid=6" | check_jsuccess
curl_getj "action.php?query=poll.kiosk&address=$KIOSK1" | expect_one standings.kiosk
curl_getj "action.php?query=poll.kiosk&address=$KIOSK2" | expect_one derbynet.kiosk
curl_getj "action.php?query=poll.kiosk&address=$KIOSK3" | expect_one feedback.kiosk
