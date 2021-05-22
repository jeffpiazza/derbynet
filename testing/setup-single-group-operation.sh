#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

`dirname $0`/reset-database.sh "$BASE_URL"

curl_postj action.php "action=json.racer.import&firstname=Adolfo \"Dolf\"&lastname=Asher&carnumber=101" | check_jsuccess
curl_postj action.php "action=json.racer.import&firstname=Angelo&lastname=Alas&carnumber=202" | check_jsuccess
curl_postj action.php "action=json.racer.import&firstname=Antoine&lastname=Akiyama&&carnumber=303" | check_jsuccess
curl_postj action.php "action=json.racer.import&firstname=Arden&lastname=Aziz&carnumber=504" | check_jsuccess
curl_postj action.php "action=json.racer.import&firstname=Barney&lastname=Bainter&carnumber=405" | check_jsuccess
curl_postj action.php "action=json.racer.import&firstname=Ben&lastname=Bittinger&carnumber=106" | check_jsuccess
curl_postj action.php "action=json.racer.import&first-last=Blake Burling&carnumber=207" | check_jsuccess
curl_postj action.php "action=json.racer.import&first-last= Bruce  Boissonneault &carnumber=308" | check_jsuccess
curl_postj action.php "action=json.racer.import&firstname=Byron&lastname=Billy&carnumber=509" | check_jsuccess
curl_postj action.php "action=json.racer.import&first-last=Craney, Carey&carnumber=410" | check_jsuccess
curl_postj action.php "action=json.racer.import&first-last=Cybulski,Carroll&carnumber=111" | check_jsuccess
curl_postj action.php "action=json.racer.import&firstname=Christoper&lastname=Chauncey&carnumber=212" | check_jsuccess
curl_postj action.php "action=json.racer.import&firstname=Clark&lastname=Chesnutt&carnumber=313" | check_jsuccess
curl_postj action.php "action=json.racer.import&firstname=Cletus&lastname=Creager&carnumber=514" | check_jsuccess


curl_postj action.php "action=json.racer.pass&racer=1&value=1" | check_jsuccess
curl_postj action.php "action=json.racer.pass&racer=2&value=1" | check_jsuccess
curl_postj action.php "action=json.racer.pass&racer=3&value=1" | check_jsuccess
curl_postj action.php "action=json.racer.pass&racer=4&value=1" | check_jsuccess
curl_postj action.php "action=json.racer.pass&racer=5&value=1" | check_jsuccess
curl_postj action.php "action=json.racer.pass&racer=6&value=1" | check_jsuccess
curl_postj action.php "action=json.racer.pass&racer=7&value=1" | check_jsuccess
curl_postj action.php "action=json.racer.pass&racer=8&value=1" | check_jsuccess
curl_postj action.php "action=json.racer.pass&racer=9&value=1" | check_jsuccess
curl_postj action.php "action=json.racer.pass&racer=10&value=1" | check_jsuccess
curl_postj action.php "action=json.racer.pass&racer=11&value=1" | check_jsuccess
curl_postj action.php "action=json.racer.pass&racer=12&value=1" | check_jsuccess
curl_postj action.php "action=json.racer.pass&racer=13&value=1" | check_jsuccess
curl_postj action.php "action=json.racer.pass&racer=14&value=1" | check_jsuccess
#curl_postj action.php "action=json.racer.pass&racer=15&value=1" | check_jsuccess

curl_postj action.php "action=json.settings.write&unused-lane-mask=0&n-lanes=4" | check_jsuccess

curl_postj action.php "action=json.schedule.generate&roundid=1" | check_jsuccess

curl_postj action.php "action=json.heat.select&roundid=1&now_racing=1" | check_jsuccess
