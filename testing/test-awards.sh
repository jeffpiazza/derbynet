#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

if [ "$2" = "basic" ] ; then
    # Basic racing
    SECOND_TEST=' firstname="Kris"'
    THIRD_TEST=' lastname="Burling"'
else
    # Master schedule racing
    SECOND_TEST=' firstname="Ben"'
    THIRD_TEST=' lastname="Halfacre"'
fi

user_login_coordinator

curl_post action.php "action=award.import" | check_failure
curl_post action.php "action=award.import&awardname=Test%20Award" | check_failure
curl_post action.php "action=award.import&awardname=Test%20Award&awardtype=Design" | check_failure
curl_post action.php "action=award.import&awardname=Test%20Award&awardtype=Design%20Trophy" | check_success
curl_post action.php "action=award.import&awardname=Test%20Den%20Award&classname=Hyenas&awardtype=Design%20Trophy" | check_failure
curl_post action.php "action=award.import&awardname=Test%20Den%20Award&classname=White's%20Wolves&awardtype=Design%20Trophy" | check_success
curl_post action.php "action=award.import&awardname=Test%20Rank%20Award&subgroup=White's%20Wolves&awardtype=Design%20Trophy&racerid=35" | check_success
curl_post action.php "action=award.import&awardname=Test%20Pack%20Award&awardtype=Design%20Trophy&carnumber=222&sort=6" | check_success

# The numbering of awardids depends critically on what awards were already
# present when the award was registered.
curl_post action.php "action=award.present&key=award-3" | check_success
curl_get "action.php?query=award.current" | expect_count '<award ' 1
curl_get "action.php?query=award.current" | expect_count 'Howell' 1
curl_get "action.php?query=award.current" | expect_count ' reveal="false"' 1

curl_get "action.php?query=award.list" | expect_count '<award ' 4
curl_get "action.php?query=award.list" | grep 'awardid="3" ' | expect_one 'racerid="35"'
curl_get "action.php?query=award.list" | grep '<award ' | expect_count 'awardtypeid="4"' 4
curl_get "action.php?query=award.list" | grep 'awardid="4" ' | expect_one 'racerid="22"'

curl_post action.php "action=award.winner&awardid=1&racerid=2" | check_success
curl_post action.php "action=award.edit&awardid=2&class_and_rank=3,3" | check_success
curl_post action.php "action=award.edit&awardid=3&name=Third" | check_success
curl_post action.php "action=award.edit&awardid=4&awardtypeid=2" | check_success

curl_get "action.php?query=award.list" | expect_count '<award ' 4
curl_get "action.php?query=award.list" | grep 'awardid="1" ' | expect_one 'racerid="2"'
curl_get "action.php?query=award.list" | expect_one 'racerid="2"'
curl_get "action.php?query=award.list" | expect_one 'carnumber="202"'
curl_get "action.php?query=award.list" | grep 'awardid="2" ' | expect_one 'classid="3"'
curl_get "action.php?query=award.list" | grep 'awardid="2" ' | expect_one 'rankid="3"'
curl_get "action.php?query=award.list" | grep '<award ' | expect_one 'classid="3"'
curl_get "action.php?query=award.list" | grep '<award ' | expect_one 'rankid="3"'
curl_get "action.php?query=award.list" | grep 'awardid="3" ' | expect_one 'racerid="35"'
curl_get "action.php?query=award.list" | grep 'awardid="3" ' | expect_one 'awardname="Third"'
curl_get "action.php?query=award.list" | grep 'awardid="4" ' | expect_one 'racerid="22"'
curl_get "action.php?query=award.list" | grep 'awardid="4" ' | expect_one 'awardtypeid="2"'

curl_post action.php "action=award.edit&awardid=2&class_and_rank=0,0" | check_success
curl_get "action.php?query=award.list" | grep 'awardid="2"' | expect_one 'classid="0"'

curl_post action.php "action=award.delete&awardid=3" | expect_count '<award ' 3
curl_post action.php "action=award.edit&awardid=new&awardtypeid=4&name=NewThird" | expect_count '<award ' 4
curl_get "action.php?query=award.list" | grep 'awardid="5"' | expect_one 'sort="7"'
curl_post action.php "action=award.order&awardid_1=4&awardid_2=5&awardid_3=2&awardid_4=1" | grep 'awardid="4"' | expect_one 'sort="1"'
curl_get "action.php?query=award.list" | grep 'awardid="5"' | expect_one 'sort="2"'
curl_get "action.php?query=award.list" | grep 'awardid="2"' | expect_one 'sort="3"'
curl_get "action.php?query=award.list" | grep 'awardid="1"' | expect_one 'sort="4"'

# Try some ad-hoc awards
# 21 = Derek Dreier, car 121
# 12 = Christopher Chauncey, car 212
curl_post action.php "action=award.adhoc&racerid=21&awardname=Best%20Use%20Of%20Chocolate" | check_success
curl_post action.php "action=award.adhoc&racerid=12&awardname=Most%20Glittery" | check_success
curl_post action.php "action=award.adhoc&racerid=21&awardname=" | check_success

curl_get "action.php?query=award.list" | expect_count 'Chocolate' 0
curl_get "action.php?query=award.list" | expect_count 'racerid=12' 0
curl_get "action.php?query=award.list" | grep 'Glittery' | expect_one 'racerid="12"'
curl_get "action.php?query=award.list&adhoc=0" | expect_count 'Glittery' 0

# There's no current award at the very beginning, but we don't want to enforce
# that this test has to be run at the very beginning.
#
# curl_get "action.php?query=award.current" | check_failure

curl_post action.php "action=award.present&key=speed-2" | check_success
curl_get "action.php?query=award.current" | expect_count '<award ' 1
curl_get "action.php?query=award.current" | expect_count ' reveal="false"' 1

curl_post action.php "action=award.present&reveal=1" | check_success
curl_get "action.php?query=award.current" | expect_count '<award ' 1
curl_get "action.php?query=award.current" | expect_count ' reveal="true"' 1
curl_get "action.php?query=award.current" | expect_count "$SECOND_TEST" 1

curl_post action.php "action=award.present&reveal=0&key=speed-3" | check_success
curl_get "action.php?query=award.current" | expect_count '<award ' 1
curl_get "action.php?query=award.current" | expect_count ' reveal="false"' 1
curl_get "action.php?query=award.current" | expect_count "$THIRD_TEST" 1

