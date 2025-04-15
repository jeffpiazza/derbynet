#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

if [ "$2" = "basic" ] ; then
    # Basic racing
    SECOND_TEST="Kris Kaba"
    THIRD_TEST="Blake Burling"
else
    # Master schedule racing
    SECOND_TEST="Ben Bittinger"
    THIRD_TEST="Herb Halfacre"
fi

# This test depends on the set-up and simulated racing from other tests.

user_login_coordinator

curl_postj action.php "action=award.import" | check_jfailure
curl_postj action.php "action=award.import&awardname=Test%20Award" | check_jfailure
# curl_postj action.php "action=award.import&awardname=Test%20Award&awardtype=Design" | check_jfailure
curl_postj action.php "action=award.import&awardname=Test%20Award&awardtype=Design%20Trophy" | check_jsuccess
curl_postj action.php "action=award.import&awardname=Test%20Den%20Award&classname=Hyenas&awardtype=Design%20Trophy" | check_jfailure
curl_postj action.php "action=award.import&awardname=Test%20Den%20Award&classname=White's%20Wolves&awardtype=Design%20Trophy" | check_jsuccess
curl_postj action.php "action=award.import&awardname=Test%20Rank%20Award&subgroup=White's%20Wolves&awardtype=Design%20Trophy&racerid=35" | check_jsuccess
curl_postj action.php "action=award.import&awardname=Test%20Pack%20Award&awardtype=Design%20Trophy&carnumber=222&sort=6" | check_jsuccess

# The numbering of awardids depends critically on what awards were already
# present when the award was registered.
curl_postj action.php "action=award.present&key=award-3" | check_jsuccess
curl_getj "action.php?query=award.current" | \
    jq '(.current.reveal | not) and (.current.recipient | test("Howell.*"))' | expect_eq true

curl_getj "action.php?query=award.list" | jq '.awards | length' | expect_eq 4
curl_getj "action.php?query=award.list" | \
    jq '.awards | map(select(.awardid == 3)) | length == 1 and .[0].racerid == 35' | \
    expect_eq true
curl_getj "action.php?query=award.list" | \
    jq '.awards | map(select(.awardtypeid == 4)) | length' | \
    expect_eq 4
curl_getj "action.php?query=award.list" | \
    jq '.awards | map(select(.awardid == 4)) | .[] | .racerid == 22' | \
    expect_eq true

curl_postj action.php "action=racer.edit&racer=11&firstname=Carroll&lastname=Cybulski&carno=111&carname=Vroom&rankid=1&exclude=1" | check_jsuccess
curl_postj action.php "action=award.winner&awardid=1&racerid=11" | check_jfailure
curl_postj action.php "action=award.winner&awardid=1&racerid=2" | check_jsuccess
curl_postj action.php "action=award.winner&awardid=1&racerid=0" | check_jsuccess
curl_postj action.php "action=award.winner&awardid=1&racerid=2" | check_jsuccess

curl_postj action.php "action=award.edit&awardid=2&class_and_rank=3,3" | check_jsuccess
curl_postj action.php "action=award.edit&awardid=3&name=Third" | check_jsuccess
curl_postj action.php "action=award.edit&awardid=4&awardtypeid=2" | check_jsuccess

curl_postj action.php "action=award.winner&awardid=2&racerid=2" | check_jfailure

curl_getj "action.php?query=award.list" | jq '.awards | length' | expect_eq 4
curl_getj "action.php?query=award.list" | jq '.awards | map(select(.classid == 3)) | length' | expect_eq 1
curl_getj "action.php?query=award.list" | jq '.awards | map(select(.rankid == 3)) | length' | expect_eq 1
curl_getj "action.php?query=award.list" | \
    jq '.awards | map(select(.awardid == 1)) | 
        length == 1 and .[0].racerid == 2' | \
    expect_eq true
curl_getj "action.php?query=award.list" | \
    jq '.awards | map(select(.awardid == 2)) | 
        length == 1 and .[0].classid == 3 and .[0].rankid == 3' | \
    expect_eq true
curl_getj "action.php?query=award.list" | \
    jq '.awards | map(select(.awardid == 3)) | 
        length == 1 and .[0].racerid == 35 and .[0].awardname == "Third"' | \
    expect_eq true
curl_getj "action.php?query=award.list" | \
    jq '.awards | map(select(.awardid == 4)) | 
        length == 1 and .[0].racerid == 22 and .[0].awardtypeid == 2' | \
    expect_eq true

curl_postj action.php "action=award.edit&awardid=2&class_and_rank=0,0" | check_jsuccess
curl_getj "action.php?query=award.list" | \
    jq '.awards | map(select(.awardid == 2)) | length == 1 and .[0].classid == 0' | \
    expect_eq true

curl_postj action.php "action=award.delete&awardid=3" | \
    jq '.awards | length' | expect_eq 3
curl_postj action.php "action=award.edit&awardid=new&awardtypeid=4&name=NewThird" | \
    jq '.awards | length' | expect_eq 4
curl_getj "action.php?query=award.list" | \
    jq '.awards | map(select(.awardid == 5)) | length == 1 and .[0].sort == 7' | \
    expect_eq true
curl_postj action.php "action=award.order&awardid_1=4&awardid_2=5&awardid_3=2&awardid_4=1" | \
    jq '.awards | map(select(.awardid == 4)) | length == 1 and .[0].sort == 1' | \
    expect_eq true

curl_getj "action.php?query=award.list" | \
    jq '.awards | all((.awardid == 4 and .sort == 1) or
                      (.awardid == 5 and .sort == 2) or
                      (.awardid == 2 and .sort == 3) or
                      (.awardid == 1 and .sort == 4))' | \
                          expect_eq true

## Create "Younger" aggregate class of classes 1,2.  Should give classid=8
curl_postj action.php "action=class.add&constituent_1=1&constituent_2=1&name=Younger" | check_jsuccess
## Create an award for the aggregate class.  Award rankid won't matter for an aggregate class award.
curl_postj action.php "action=award.edit&awardid=new&awardtypeid=2&name=AggClass%20Award&class_and_rank=8,0" | check_jsuccess

curl_postj action.php "action=award.winner&awardid=6&racerid=48" | check_jfailure
curl_postj action.php "action=award.winner&awardid=6&racerid=1" | check_jsuccess

## Create an empty subgroup for Lions & Tigers
curl_postj action.php "action=settings.write&do-use-subgroups=1&do-use-subgroups-checkbox=1" | check_jsuccess
curl_postj action.php "action=rank.add&name=Private&classid=1" | check_jsuccess
## Create an "aggregate" class comprising only the new Private rankid
curl_postj action.php "action=class.add&rankid_8=1&name=AggExclusive" | check_jsuccess
## Create an award for that exclusive (empty) class
curl_postj action.php "action=award.edit&awardid=new&awardtypeid=2&name=Exclusive&class_and_rank=1,8" | check_jsuccess

curl_postj action.php "action=award.winner&awardid=7&racerid=1" | check_jfailure
# Move racer 1 to the previously empty subgroup and now the award can be granted
curl_postj action.php "action=racer.edit&racer=1&rankid=8" | check_jsuccess
curl_postj action.php "action=award.winner&awardid=7&racerid=1" | check_jsuccess

# Try some ad-hoc awards
# 21 = Derek Dreier, car 121
# 12 = Christopher Chauncey, car 212
curl_postj action.php "action=award.adhoc&racerid=21&awardname=Best%20Use%20Of%20Chocolate" | check_jsuccess
curl_postj action.php "action=award.adhoc&racerid=12&awardname=Most%20Glittery" | check_jsuccess
curl_postj action.php "action=award.adhoc&racerid=21&awardname=" | check_jsuccess

curl_getj "action.php?query=award.list" | expect_count 'Chocolate' 0
curl_getj "action.php?query=award.list" | \
    jq '.awards | map(select(.racerid == 12)) | length == 1 and .[0].awardname == "Most Glittery" and .[0].adhoc' | \
    expect_eq true

# Ad hoc awards aren't supposed to affect numbering for named awards
curl_postj action.php "action=award.edit&awardid=new&awardtypeid=4&name=NewFourth" | check_jsuccess
curl_getj "action.php?query=award.list" | \
    jq '.awards | map(select(.awardname == "NewFourth")) | length == 1 and .[0].sort == 7' | \
    expect_eq true

# There's no current award at the very beginning, but we don't want to enforce
# that this test has to be run at the very beginning.
#
# curl_getj "action.php?query=award.current" | check_jfailure

# The presence of two aggregate classes would make second-fastest-in-pack (speed-2)
# not meaningful.
curl_postj action.php "action=class.delete&classid=9" | check_jsuccess
curl_postj action.php "action=class.delete&classid=8" | check_jsuccess

curl_postj action.php "action=award.present&key=speed-2" | check_jsuccess
curl_getj "action.php?query=award.current" | \
    jq '.current.reveal' | expect_eq false

curl_postj action.php "action=award.present&reveal=1" | check_jsuccess
curl_getj "action.php?query=award.current" | \
    jq --arg R "$SECOND_TEST" '.current.reveal and .current.recipient == $R' | \
    expect_eq true

curl_postj action.php "action=award.present&reveal=0&key=speed-3" | check_jsuccess
curl_getj "action.php?query=award.current" | \
    jq --arg R "$THIRD_TEST" '(.current.reveal | not) and .current.recipient == $R' | \
    expect_eq true
