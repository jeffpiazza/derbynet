#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

PHOTO_DIR=/Users/jeffpiazza/test-photos

if [ ! `echo "$BASE_URL" | grep -i localhost` ]; then
    tput setaf 2  # green text
    echo Skipping photo assignment tests
    tput setaf 0  # black text
    exit 0
fi

if [ ! -d "$PHOTO_DIR" ]; then
    tput setaf 2  # green text
    echo Skipping photo assignment tests
    tput setaf 0  # black text
    exit 0
fi

`dirname $0`/login-coordinator.sh $BASE_URL

curl_post action.php "action=write-settings&photo-dir=$PHOTO_DIR" | check_success
curl_post action.php "action=write-settings&n-lanes=4" | check_success
curl_post action.php "action=write-settings&show-racer-photos=1&show-racer-photos-checkbox=1" | check_success

curl_post action.php "action=photo.assign&racer=1&photo=IMG_5392.jpg" | check_success
curl_post action.php "action=photo.assign&racer=2&photo=IMG_5393.jpg" | check_success
curl_post action.php "action=photo.assign&racer=3&photo=Jack%20Collins.jpg" | check_success
curl_post action.php "action=photo.assign&racer=4&photo=audi-g.jpg" | check_success
curl_post action.php "action=photo.assign&racer=5&photo=bender-c.jpg" | check_success
curl_post action.php "action=photo.assign&racer=6&photo=berg-a.jpg" | check_success
curl_post action.php "action=photo.assign&racer=7&photo=berg-m.jpg" | check_success
curl_post action.php "action=photo.assign&racer=8&photo=berg-p.jpg" | check_success
curl_post action.php "action=photo.assign&racer=9&photo=cahaly-j.jpg" | check_success
curl_post action.php "action=photo.assign&racer=10&photo=collins-j.jpg" | check_success
curl_post action.php "action=photo.assign&racer=11&photo=ditelberg-s.jpg" | check_success
curl_post action.php "action=photo.assign&racer=12&photo=doe-b.jpg" | check_success
curl_post action.php "action=photo.assign&racer=13&photo=eburne-n.jpg" | check_success
curl_post action.php "action=photo.assign&racer=14&photo=fantasia-a.jpg" | check_success
curl_post action.php "action=photo.assign&racer=15&photo=fantasia-j.jpg" | check_success
curl_post action.php "action=photo.assign&racer=16&photo=gallagher-g.jpg" | check_success
curl_post action.php "action=photo.assign&racer=17&photo=gareau-m.jpg" | check_success
curl_post action.php "action=photo.assign&racer=18&photo=hillmer-r.jpg" | check_success
curl_post action.php "action=photo.assign&racer=19&photo=hillmer-t.jpg" | check_success
curl_post action.php "action=photo.assign&racer=20&photo=horner-c.jpg" | check_success
curl_post action.php "action=photo.assign&racer=21&photo=horner-h.jpg" | check_success
curl_post action.php "action=photo.assign&racer=22&photo=houghton-g.jpg" | check_success
curl_post action.php "action=photo.assign&racer=23&photo=howell-b.jpg" | check_success
curl_post action.php "action=photo.assign&racer=24&photo=lehmann-r.jpg" | check_success
curl_post action.php "action=photo.assign&racer=25&photo=mackinnon-h.jpg" | check_success
curl_post action.php "action=photo.assign&racer=26&photo=mealey-c.jpg" | check_success
curl_post action.php "action=photo.assign&racer=27&photo=mendes-f.jpg" | check_success
curl_post action.php "action=photo.assign&racer=28&photo=meyers-j.jpg" | check_success
curl_post action.php "action=photo.assign&racer=29&photo=mirick-b.jpg" | check_success
curl_post action.php "action=photo.assign&racer=30&photo=muenning-j.jpg" | check_success
curl_post action.php "action=photo.assign&racer=31&photo=mui-d.jpg" | check_success
curl_post action.php "action=photo.assign&racer=32&photo=natoli-j.jpg" | check_success
curl_post action.php "action=photo.assign&racer=33&photo=smith-c.jpg" | check_success
curl_post action.php "action=photo.assign&racer=34&photo=tellalian-e.jpg" | check_success
curl_post action.php "action=photo.assign&racer=35&photo=todorov-m.jpg" | check_success
curl_post action.php "action=photo.assign&racer=36&photo=topple-r.jpg" | check_success

curl_post action.php "action=photo.assign&racer=37&photo=Aiden%20Moore.jpg" | check_success
curl_post action.php "action=photo.assign&racer=38&photo=Alex%20Dehn.jpg" | check_success
curl_post action.php "action=photo.assign&racer=39&photo=Alex%20Ma.jpg" | check_success
curl_post action.php "action=photo.assign&racer=40&photo=Alex%20Perkin.jpg" | check_success
curl_post action.php "action=photo.assign&racer=41&photo=Alex%20Perkin%202.jpg" | check_success
curl_post action.php "action=photo.assign&racer=42&photo=Andrew%20Ho.jpg" | check_success
curl_post action.php "action=photo.assign&racer=43&photo=Andrew%20Martin.jpg" | check_success
curl_post action.php "action=photo.assign&racer=44&photo=Ben%20Dehn.jpg" | check_success
curl_post action.php "action=photo.assign&racer=45&photo=Ben%20Dehn-2.jpg" | check_success
curl_post action.php "action=photo.assign&racer=46&photo=Ben%20Mirick.jpg" | check_success
curl_post action.php "action=photo.assign&racer=47&photo=Blake%20Foster.jpg" | check_success
curl_post action.php "action=photo.assign&racer=48&photo=Burt%20Ford.jpg" | check_success
curl_post action.php "action=photo.assign&racer=49&photo=Cameron%20Morken.jpg" | check_success
curl_post action.php "action=photo.assign&racer=50&photo=Carter%20Smith.jpg" | check_success
curl_post action.php "action=photo.assign&racer=51&photo=Connor%20Roy.jpg" | check_success
curl_post action.php "action=photo.assign&racer=52&photo=Cooper%20Reisner.jpg" | check_success
curl_post action.php "action=photo.assign&racer=53&photo=Devon%20Escobar.jpg" | check_success
curl_post action.php "action=photo.assign&racer=54&photo=Dylan%20Reepmeyer.jpg" | check_success
curl_post action.php "action=photo.assign&racer=55&photo=Evan%20LaRiviere.jpg" | check_success
curl_post action.php "action=photo.assign&racer=56&photo=Gary%20Martin.jpg" | check_success
curl_post action.php "action=photo.assign&racer=57&photo=Gaurav%20Capila.jpg" | check_success
curl_post action.php "action=photo.assign&racer=58&photo=Hayden%20Morken.jpg" | check_success
curl_post action.php "action=photo.assign&racer=59&photo=Henry%20Mackinnon.jpg" | check_success
curl_post action.php "action=photo.assign&racer=60&photo=IMG_5050.jpg" | check_success
curl_post action.php "action=photo.assign&racer=61&photo=Jace%20Watson.jpg" | check_success
curl_post action.php "action=photo.assign&racer=62&photo=Jack%20Collins.jpg" | check_success
curl_post action.php "action=photo.assign&racer=63&photo=Jacob%20Koppel.jpg" | check_success
curl_post action.php "action=photo.assign&racer=64&photo=Jaime%20Duffy.jpg" | check_success
curl_post action.php "action=photo.assign&racer=65&photo=James%20Castle.jpg" | check_success
curl_post action.php "action=photo.assign&racer=66&photo=Jase%20Watson.jpg" | check_success
curl_post action.php "action=photo.assign&racer=67&photo=Joey%20McNeil.jpg" | check_success
curl_post action.php "action=photo.assign&racer=68&photo=Jonathan%20Chiu.jpg" | check_success
curl_post action.php "action=photo.assign&racer=69&photo=Liam%20Donlan.jpg" | check_success
curl_post action.php "action=photo.assign&racer=70&photo=Marco%20Todorov.jpg" | check_success
curl_post action.php "action=photo.assign&racer=71&photo=Matheos%20Zarou.jpg" | check_success
curl_post action.php "action=photo.assign&racer=72&photo=Matt%20DeVito.jpg" | check_success
curl_post action.php "action=photo.assign&racer=73&photo=Matthew%20Johnson.jpg" | check_success
curl_post action.php "action=photo.assign&racer=74&photo=Matthew%20Klocke.jpg" | check_success
curl_post action.php "action=photo.assign&racer=75&photo=Matthew%20Leibman.jpg" | check_success
curl_post action.php "action=photo.assign&racer=76&photo=Michael%20Skuratovsky.jpg" | check_success
curl_post action.php "action=photo.assign&racer=77&photo=Quinn%20Fallon.jpg" | check_success
curl_post action.php "action=photo.assign&racer=78&photo=Robert%20Foster.jpg" | check_success
curl_post action.php "action=photo.assign&racer=79&photo=Will%20Webster.jpg" | check_success
curl_post action.php "action=photo.assign&racer=80&photo=Will%20Workman.jpg" | check_success
curl_post action.php "action=photo.assign&racer=81&photo=Zack%20Bender.jpg" | check_success
curl_post action.php "action=photo.assign&racer=82&photo=Zack%20DuPont.jpg" | check_success
# curl_post action.php "action=photo.assign&racer=83&photo=Grayson%20Houghton.jpg" | check_success
# curl_post action.php "action=photo.assign&racer=84&photo=Matthew%20Klocke-2.jpg" | check_success



