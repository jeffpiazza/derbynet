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

curl_post action.php "action=racer.edit&racer=11&firstname=Carroll&lastname=Cybulski&carno=111&carname=Vroom&rankid=1&exclude=1" | check_success
curl_post action.php "action=award.winner&awardid=1&racerid=11" | check_failure
curl_post action.php "action=award.winner&awardid=1&racerid=2" | check_success
curl_post action.php "action=award.winner&awardid=1&racerid=0" | check_success
curl_post action.php "action=award.winner&awardid=1&racerid=2" | check_success

curl_post action.php "action=award.edit&awardid=2&class_and_rank=3,3" | check_success
curl_post action.php "action=award.edit&awardid=3&name=Third" | check_success
curl_post action.php "action=award.edit&awardid=4&awardtypeid=2" | check_success

curl_post action.php "action=award.winner&awardid=2&racerid=2" | check_failure

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

## Create "Younger" aggregate class of classes 1,2.  Should give classid=8
curl_post action.php "action=class.add&constituent_1=1&constituent_2=1&name=Younger" | check_success
## Create an award for the aggregate class.  Award rankid won't matter for an aggregate class award.
curl_post action.php "action=award.edit&awardid=new&awardtypeid=2&name=AggClass%20Award&class_and_rank=8,8" | check_success

curl_post action.php "action=award.winner&awardid=6&racerid=48" | check_failure
curl_post action.php "action=award.winner&awardid=6&racerid=1" | check_success

## Create an empty subgroup for Lions & Tigers
curl_post action.php "action=settings.write&do-use-subgroups=1&do-use-subgroups-checkbox=1" | check_success
curl_post action.php "action=rank.add&name=Private&classid=1" | check_success
## Create an "aggregate" class comprising only the new Private rankid
curl_post action.php "action=class.add&rankid_8=1&name=AggExclusive" | check_success
## Create an award for that exclusive (empty) class
curl_post action.php "action=award.edit&awardid=new&awardtypeid=2&name=Exclusive&class_and_rank=1,8" | check_success

curl_post action.php "action=award.winner&awardid=7&racerid=1" | check_failure
# Move racer 1 to the previously empty subgroup and now the award can be granted
curl_post action.php "action=racer.edit&racer=1&rankid=8" | check_success
curl_post action.php "action=award.winner&awardid=7&racerid=1" | check_success

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

# Ad hoc awards aren't supposed to affect numbering for named awards
curl_post action.php "action=award.edit&awardid=new&awardtypeid=4&name=NewFourth" | check_success
curl_get "action.php?query=award.list" | grep NewFourth | expect_one 'sort="7"'

# There's no current award at the very beginning, but we don't want to enforce
# that this test has to be run at the very beginning.
#
# curl_get "action.php?query=award.current" | check_failure

# The presence of two aggregate classes would make second-fastest-in-pack (speed-2)
# not meaningful.
curl_post action.php "action=class.delete&classid=9" | check_success
curl_post action.php "action=class.delete&classid=8" | check_success

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

