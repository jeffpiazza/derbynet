#! /bin/sh

BASE_URL=$1
source `dirname $0`/common.sh

curl_post action.php "action=pass&racer=1&value=1" | check_success
curl_post action.php "action=pass&racer=3&value=1" | check_success
curl_post action.php "action=pass&racer=5&value=1" | check_success
curl_post action.php "action=pass&racer=7&value=1" | check_success
curl_post action.php "action=pass&racer=9&value=1" | check_success
curl_post action.php "action=pass&racer=11&value=1" | check_success
curl_post action.php "action=pass&racer=13&value=1" | check_success
curl_post action.php "action=pass&racer=15&value=1" | check_success
curl_post action.php "action=pass&racer=17&value=1" | check_success
#curl_post action.php "action=pass&racer=19&value=1" | check_success
curl_post action.php "action=pass&racer=21&value=1" | check_success
curl_post action.php "action=pass&racer=23&value=1" | check_success
curl_post action.php "action=pass&racer=25&value=1" | check_success
curl_post action.php "action=pass&racer=27&value=1" | check_success
#curl_post action.php "action=pass&racer=29&value=1" | check_success
curl_post action.php "action=pass&racer=31&value=1" | check_success
curl_post action.php "action=pass&racer=33&value=1" | check_success
curl_post action.php "action=pass&racer=35&value=1" | check_success
curl_post action.php "action=pass&racer=37&value=1" | check_success
curl_post action.php "action=pass&racer=39&value=1" | check_success
curl_post action.php "action=pass&racer=41&value=1" | check_success
curl_post action.php "action=pass&racer=43&value=1" | check_success
curl_post action.php "action=pass&racer=45&value=1" | check_success
curl_post action.php "action=pass&racer=47&value=1" | check_success
curl_post action.php "action=pass&racer=49&value=1" | check_success

# Exclude car 111
curl_post action.php "action=edit-racer&racer=11&firstname=Carroll&lastname=Cybulski&carno=111&rankid=1&exclude=1" | check_success

