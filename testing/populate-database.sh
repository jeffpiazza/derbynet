#! /bin/sh
BASE_URL=$1

source common.sh


curl_post action.php "action=new-racer&carno=301&lastname=Adrogue&firstname=Nico&rankid=3" | expect_one '"lastname-1"'
curl_post action.php "action=new-racer&carno=501&lastname=Adrogue&firstname=Tommy&rankid=5" | expect_one '"lastname-2"'
curl_post action.php "action=new-racer&carno=101&lastname=Bender&firstname=Cole&rankid=1" | expect_one '"lastname-3"'
curl_post action.php "action=new-racer&carno=302&lastname=Canino&firstname=Christopher&rankid=3" | expect_one '"lastname-4"'
curl_post action.php "action=new-racer&carno=201&lastname=Fouda&firstname=Effa&rankid=2" | expect_one '"lastname-5"'
curl_post action.php "action=new-racer&carno=502&lastname=Fouda&firstname=Ottou&rankid=5" | expect_one '"lastname-6"'
curl_post action.php "action=new-racer&carno=401&lastname=Ciolfi&firstname=John&rankid=4" | expect_one '"lastname-7"'
curl_post action.php "action=new-racer&carno=102&lastname=Colone&firstname=Ryan&rankid=1" | expect_one '"lastname-8"'
curl_post action.php "action=new-racer&carno=402&lastname=Czubarow&firstname=Tony&rankid=4" | expect_one '"lastname-9"'
curl_post action.php "action=new-racer&carno=202&lastname=DePeyster&firstname=Rowan&rankid=2" > /dev/null
curl_post action.php "action=new-racer&carno=203&lastname=Eburne&firstname=Nicholas&rankid=2" > /dev/null
curl_post action.php "action=new-racer&carno=403&lastname=Erickson&firstname=Simon&rankid=4" > /dev/null
curl_post action.php "action=new-racer&carno=103&lastname=Fantasia&firstname=Andrew&rankid=1" > /dev/null
curl_post action.php "action=new-racer&carno=104&lastname=Fantasia&firstname=Joseph&rankid=1" > /dev/null
curl_post action.php "action=new-racer&carno=105&lastname=Fichtel&firstname=Andrew&rankid=1" > /dev/null
curl_post action.php "action=new-racer&carno=204&lastname=Fletcher&firstname=Holt&rankid=2" > /dev/null
curl_post action.php "action=new-racer&carno=205&lastname=Gallagher&firstname=Gavin&rankid=2" > /dev/null
curl_post action.php "action=new-racer&carno=508&lastname=Gallagher&firstname=Maia&rankid=5" > /dev/null
curl_post action.php "action=new-racer&carno=206&lastname=Garber&firstname=Sam&rankid=2" > /dev/null
curl_post action.php "action=new-racer&carno=207&lastname=Gittleman&firstname=Frank&rankid=2" | expect_one '"class-20"'
curl_post action.php "action=new-racer&carno=208&lastname=Golod&firstname=Ari&rankid=2" > /dev/null
curl_post action.php "action=new-racer&carno=404&lastname=Gomez&firstname=Julien&rankid=4" > /dev/null
curl_post action.php "action=new-racer&carno=209&lastname=Haidar&firstname=Alexander&rankid=2" > /dev/null
curl_post action.php "action=new-racer&carno=509&lastname=Hillmer&firstname=Catie&rankid=5" > /dev/null
curl_post action.php "action=new-racer&carno=303&lastname=Hillmer&firstname=John&rankid=3" > /dev/null
curl_post action.php "action=new-racer&carno=106&lastname=Hillmer&firstname=Ryan&rankid=1" > /dev/null
curl_post action.php "action=new-racer&carno=107&lastname=Hillmer&firstname=Thomas&rankid=1" > /dev/null
curl_post action.php "action=new-racer&carno=210&lastname=Jones&firstname=David&rankid=2" > /dev/null
curl_post action.php "action=new-racer&carno=108&lastname=Laing&firstname=Michael&rankid=1" > /dev/null
curl_post action.php "action=new-racer&carno=507&lastname=Lapides&firstname=Aria&rankid=5" | expect_one '"class-30"'
curl_post action.php "action=new-racer&carno=211&lastname=Lapides&firstname=Sage&rankid=2" > /dev/null
curl_post action.php "action=new-racer&carno=212&lastname=Lucas&firstname=John&rankid=2" > /dev/null
curl_post action.php "action=new-racer&carno=506&lastname=Luster&firstname=Jackson&rankid=5" > /dev/null
curl_post action.php "action=new-racer&carno=503&lastname=Malloy&firstname=Charlie&rankid=5" > /dev/null
curl_post action.php "action=new-racer&carno=304&lastname=Malloy&firstname=Mac&rankid=3" > /dev/null
curl_post action.php "action=new-racer&carno=504&lastname=Malloy&firstname=Murphy&rankid=5" > /dev/null
curl_post action.php "action=new-racer&carno=305&lastname=Markis&firstname=James&rankid=3" > /dev/null
curl_post action.php "action=new-racer&carno=405&lastname=Markis&firstname=John&rankid=4" > /dev/null
curl_post action.php "action=new-racer&carno=306&lastname=O'Connor&firstname=Owen&rankid=3" > /dev/null
curl_post action.php "action=new-racer&carno=307&lastname=Piazza&firstname=Mikey&rankid=3" | expect_one '"class-40"'
curl_post action.php "action=new-racer&carno=109&lastname=Poresky&firstname=Nathan&rankid=1" > /dev/null
curl_post action.php "action=new-racer&carno=308&lastname=Sours&firstname=James&rankid=3" > /dev/null
curl_post action.php "action=new-racer&carno=406&lastname=Stewart&firstname=Jack&rankid=4" > /dev/null
curl_post action.php "action=new-racer&carno=110&lastname=Stewart&firstname=Matthew&rankid=1" > /dev/null
curl_post action.php "action=new-racer&carno=407&lastname=Teixeira&firstname=Ben&rankid=4" > /dev/null
curl_post action.php "action=new-racer&carno=408&lastname=Teixeira&firstname=Sam&rankid=4" > /dev/null
curl_post action.php "action=new-racer&carno=111&lastname=Totonchy&firstname=Nicholas&rankid=1" > /dev/null
curl_post action.php "action=new-racer&carno=213&lastname=Vogel&firstname=Gavin&rankid=2" > /dev/null
curl_post action.php "action=new-racer&carno=409&lastname=Yasan&firstname=Alexander&rankid=4" > /dev/null
curl_post action.php "action=new-racer&carno=112&lastname=Yasan&firstname=Anthony&rankid=1" | expect_one '"class-50"'

curl_post action.php "action=pass&racer=1&value=1" | check_success check-in
curl_post action.php "action=pass&racer=3&value=1" | check_success check-in
curl_post action.php "action=pass&racer=5&value=1" | check_success check-in
curl_post action.php "action=pass&racer=7&value=1" | check_success check-in
curl_post action.php "action=pass&racer=9&value=1" | check_success check-in
curl_post action.php "action=pass&racer=11&value=1" | check_success check-in
curl_post action.php "action=pass&racer=13&value=1" | check_success check-in
curl_post action.php "action=pass&racer=15&value=1" | check_success check-in
curl_post action.php "action=pass&racer=17&value=1" | check_success check-in
curl_post action.php "action=pass&racer=19&value=1" | check_success check-in
curl_post action.php "action=pass&racer=21&value=1" | check_success check-in
curl_post action.php "action=pass&racer=23&value=1" | check_success check-in
curl_post action.php "action=pass&racer=25&value=1" | check_success check-in
curl_post action.php "action=pass&racer=27&value=1" | check_success check-in
curl_post action.php "action=pass&racer=29&value=1" | check_success check-in
curl_post action.php "action=pass&racer=31&value=1" | check_success check-in
curl_post action.php "action=pass&racer=33&value=1" | check_success check-in
curl_post action.php "action=pass&racer=35&value=1" | check_success check-in
curl_post action.php "action=pass&racer=37&value=1" | check_success check-in
curl_post action.php "action=pass&racer=39&value=1" | check_success check-in
curl_post action.php "action=pass&racer=41&value=1" | check_success check-in
curl_post action.php "action=pass&racer=43&value=1" | check_success check-in
curl_post action.php "action=pass&racer=45&value=1" | check_success check-in
curl_post action.php "action=pass&racer=47&value=1" | check_success check-in
curl_post action.php "action=pass&racer=49&value=1" | check_success check-in

curl_post action.php "action=xbs&racer=1&value=1" | check_success xbs
curl_post action.php "action=xbs&racer=2&value=1" | check_success xbs
curl_post action.php "action=xbs&racer=3&value=1" | check_success xbs
curl_post action.php "action=xbs&racer=1&value=0" | check_success xbs

curl_get checkin.php | grep -v xbs | expect_count ' checked=' 25
