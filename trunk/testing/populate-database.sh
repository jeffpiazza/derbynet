#! /bin/sh
BASE_URL=$1

function curl_get() {
	echo '     ' $1 >&2
	echo    >> output.curl
	echo $1 >> output.curl
	echo    >> output.curl
	curl --location -s -b cookies.curl -c cookies.curl $BASE_URL/$1 | tee debug.curl | xmllint --format - | tee -a output.curl
}

function curl_post() {
	echo '     post' $1 $2 >&2
	echo    >> output.curl
	echo post $1 $2 >> output.curl
	echo    >> output.curl
	curl --location -d $2 -s -b cookies.curl -c cookies.curl $BASE_URL/$1 | tee debug.curl | xmllint --format - | tee -a output.curl
}

curl_post newracer.php "carno=301&lastname=Adrogue&firstname=Nico&den=3,3" > /dev/null
curl_post newracer.php "carno=501&lastname=Adrogue&firstname=Tommy&den=5,5" > /dev/null
curl_post newracer.php "carno=101&lastname=Bender&firstname=Cole&den=1,1" > /dev/null
curl_post newracer.php "carno=302&lastname=Canino&firstname=Christopher&den=3,3" > /dev/null
curl_post newracer.php "carno=201&lastname=Fouda&firstname=Effa&den=2,2" > /dev/null
curl_post newracer.php "carno=502&lastname=Fouda&firstname=Ottou&den=5,5" > /dev/null
curl_post newracer.php "carno=401&lastname=Ciolfi&firstname=John&den=4,4" > /dev/null
curl_post newracer.php "carno=102&lastname=Colone&firstname=Ryan&den=1,1" > /dev/null
curl_post newracer.php "carno=402&lastname=Czubarow&firstname=Tony&den=4,4" > /dev/null
curl_post newracer.php "carno=202&lastname=DePeyster&firstname=Rowan&den=2,2" > /dev/null
curl_post newracer.php "carno=203&lastname=Eburne&firstname=Nicholas&den=2,2" > /dev/null
curl_post newracer.php "carno=403&lastname=Erickson&firstname=Simon&den=4,4" > /dev/null
curl_post newracer.php "carno=103&lastname=Fantasia&firstname=Andrew&den=1,1" > /dev/null
curl_post newracer.php "carno=104&lastname=Fantasia&firstname=Joseph&den=1,1" > /dev/null
curl_post newracer.php "carno=105&lastname=Fichtel&firstname=Andrew&den=1,1" > /dev/null
curl_post newracer.php "carno=204&lastname=Fletcher&firstname=Holt&den=2,2" > /dev/null
curl_post newracer.php "carno=205&lastname=Gallagher&firstname=Gavin&den=2,2" > /dev/null
curl_post newracer.php "carno=508&lastname=Gallagher&firstname=Maia&den=5,5" > /dev/null
curl_post newracer.php "carno=206&lastname=Garber&firstname=Sam&den=2,2" > /dev/null
curl_post newracer.php "carno=207&lastname=Gittleman&firstname=Frank&den=2,2" > /dev/null
curl_post newracer.php "carno=208&lastname=Golod&firstname=Ari&den=2,2" > /dev/null
curl_post newracer.php "carno=404&lastname=Gomez&firstname=Julien&den=4,4" > /dev/null
curl_post newracer.php "carno=209&lastname=Haidar&firstname=Alexander&den=2,2" > /dev/null
curl_post newracer.php "carno=509&lastname=Hillmer&firstname=Catie&den=5,5" > /dev/null
curl_post newracer.php "carno=303&lastname=Hillmer&firstname=John&den=3,3" > /dev/null
curl_post newracer.php "carno=106&lastname=Hillmer&firstname=Ryan&den=1,1" > /dev/null
curl_post newracer.php "carno=107&lastname=Hillmer&firstname=Thomas&den=1,1" > /dev/null
curl_post newracer.php "carno=210&lastname=Jones&firstname=David&den=2,2" > /dev/null
curl_post newracer.php "carno=108&lastname=Laing&firstname=Michael&den=1,1" > /dev/null
curl_post newracer.php "carno=507&lastname=Lapides&firstname=Aria&den=5,5" > /dev/null
curl_post newracer.php "carno=211&lastname=Lapides&firstname=Sage&den=2,2" > /dev/null
curl_post newracer.php "carno=212&lastname=Lucas&firstname=John&den=2,2" > /dev/null
curl_post newracer.php "carno=506&lastname=Luster&firstname=Jackson&den=5,5" > /dev/null
curl_post newracer.php "carno=503&lastname=Malloy&firstname=Charlie&den=5,5" > /dev/null
curl_post newracer.php "carno=304&lastname=Malloy&firstname=Mac&den=3,3" > /dev/null
curl_post newracer.php "carno=504&lastname=Malloy&firstname=Murphy&den=5,5" > /dev/null
curl_post newracer.php "carno=305&lastname=Markis&firstname=James&den=3,3" > /dev/null
curl_post newracer.php "carno=405&lastname=Markis&firstname=John&den=4,4" > /dev/null
curl_post newracer.php "carno=306&lastname=O\'Connor&firstname=Owen&den=3,3" > /dev/null
curl_post newracer.php "carno=307&lastname=Piazza&firstname=Mikey&den=3,3" > /dev/null
curl_post newracer.php "carno=109&lastname=Poresky&firstname=Nathan&den=1,1" > /dev/null
curl_post newracer.php "carno=308&lastname=Sours&firstname=James&den=3,3" > /dev/null
curl_post newracer.php "carno=406&lastname=Stewart&firstname=Jack&den=4,4" > /dev/null
curl_post newracer.php "carno=110&lastname=Stewart&firstname=Matthew&den=1,1" > /dev/null
curl_post newracer.php "carno=407&lastname=Teixeira&firstname=Ben&den=4,4" > /dev/null
curl_post newracer.php "carno=408&lastname=Teixeira&firstname=Sam&den=4,4" > /dev/null
curl_post newracer.php "carno=111&lastname=Totonchy&firstname=Nicholas&den=1,1" > /dev/null
curl_post newracer.php "carno=213&lastname=Vogel&firstname=Gavin&den=2,2" > /dev/null
curl_post newracer.php "carno=409&lastname=Yasan&firstname=Alexander&den=4,4" > /dev/null
curl_post newracer.php "carno=112&lastname=Yasan&firstname=Anthony&den=1,1" > /dev/null

curl_post action.php "action=pass&racer=1&value=1" > /dev/null
curl_post action.php "action=pass&racer=3&value=1" > /dev/null
curl_post action.php "action=pass&racer=5&value=1" > /dev/null
curl_post action.php "action=pass&racer=7&value=1" > /dev/null
curl_post action.php "action=pass&racer=9&value=1" > /dev/null
curl_post action.php "action=pass&racer=11&value=1" > /dev/null
curl_post action.php "action=pass&racer=13&value=1" > /dev/null
curl_post action.php "action=pass&racer=15&value=1" > /dev/null
curl_post action.php "action=pass&racer=17&value=1" > /dev/null
curl_post action.php "action=pass&racer=19&value=1" > /dev/null
curl_post action.php "action=pass&racer=21&value=1" > /dev/null
curl_post action.php "action=pass&racer=23&value=1" > /dev/null
curl_post action.php "action=pass&racer=25&value=1" > /dev/null
curl_post action.php "action=pass&racer=27&value=1" > /dev/null
curl_post action.php "action=pass&racer=29&value=1" > /dev/null
curl_post action.php "action=pass&racer=31&value=1" > /dev/null
curl_post action.php "action=pass&racer=33&value=1" > /dev/null
curl_post action.php "action=pass&racer=35&value=1" > /dev/null
curl_post action.php "action=pass&racer=37&value=1" > /dev/null
curl_post action.php "action=pass&racer=39&value=1" > /dev/null
curl_post action.php "action=pass&racer=41&value=1" > /dev/null
curl_post action.php "action=pass&racer=43&value=1" > /dev/null
curl_post action.php "action=pass&racer=45&value=1" > /dev/null
curl_post action.php "action=pass&racer=47&value=1" > /dev/null
curl_post action.php "action=pass&racer=49&value=1" > /dev/null

curl_post action.php "action=xbs&racer=1&value=1" > /dev/null
curl_post action.php "action=xbs&racer=2&value=1" > /dev/null
curl_post action.php "action=xbs&racer=3&value=1" > /dev/null
curl_post action.php "action=xbs&racer=1&value=0" > /dev/null

[ `curl_get checkin.php | grep -v xbs | grep -c ' checked='` -eq 25 ] || echo Wrong number of checkins
