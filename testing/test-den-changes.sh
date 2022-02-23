#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

curl_postj action.php "action=class.add&name=TheNotLastClass" | check_jsuccess

# Regression test: class.add left the sortorder field unpopulated
curl_postj action.php "action=class.add&name=TheLastClass" | check_jsuccess

CLASS_LIST=$(mktemp /tmp/derby-class.list.XXXXX)

curl_getj "action.php?query=class.list" > $CLASS_LIST
cat $CLASS_LIST | jq '.classes | length == 7 and .[6].name == "TheLastClass"' | expect_eq true
#    test_fails New class should be sorted last

CLASSID=$(cat $CLASS_LIST | jq '.classes | map(select(.name == "TheNotLastClass"))[0].classid')
curl_postj action.php "action=class.delete&classid=$CLASSID" | check_jsuccess

CLASSID=$(cat $CLASS_LIST | jq '.classes | map(select(.name | match("Bears.*")))[0].classid')

RANKID=$(cat $CLASS_LIST | jq --argjson cl $CLASSID '.classes | map(select(.classid == $cl))[0].subgroups[0].rankid')

# Can't delete the only rank in a class
curl_postj action.php "action=rank.delete&rankid=$RANKID" | check_jfailure

curl_postj action.php "action=rank.add&classid=$CLASSID&name=SecondRank" | check_jsuccess
RANKID2=$(curl_getj "action.php?query=class.list" | \
              jq '.classes | map(.subgroups | map(select(.name == "SecondRank"))[0].rankid) |
                             map(select(. != null))[0]')

curl_postj action.php "action=rank.order&rankid_1=$RANKID2&rankid_2=$RANKID" | check_jsuccess
curl_postj action.php "action=rank.edit&rankid=$RANKID2&name=New%20Rank%20Name" | check_jsuccess

GOLDEN=$(mktemp /tmp/derby-golden-class.list.XXXXX)
cat >$GOLDEN <<EOF
{
    "classes": [
        {
            "classid": 1,
            "count": 17,
            "nrounds": 0,
            "ntrophies": -1,
            "name": "Lions & Tigers",
            "subgroups": [
                {
                    "rankid": 1,
                    "count": 17,
                    "name": "Lions & Tigers"
                }
            ]
        },
        {
            "classid": 2,
            "count": 17,
            "nrounds": 0,
            "ntrophies": -1,
            "name": "White's Wolves",
            "subgroups": [
                {
                    "rankid": 2,
                    "count": 17,
                    "name": "White's Wolves"
                }
            ]
        },
        {
            "classid": 3,
            "count": 16,
            "nrounds": 0,
            "ntrophies": -1,
            "name": "Bears and Fr\u00e8res",
            "subgroups": [
                {
                    "rankid": 8,
                    "count": 0,
                    "name": "New Rank Name"
                },
                {
                    "rankid": 3,
                    "count": 16,
                    "name": "Bears and Fr\u00e8res"
                }
            ]
        },
        {
            "classid": 4,
            "count": 16,
            "nrounds": 0,
            "ntrophies": -1,
            "name": "Webelos (\"Webes\")",
            "subgroups": [
                {
                    "rankid": 4,
                    "count": 16,
                    "name": "Webelos (\"Webes\")"
                }
            ]
        },
        {
            "classid": 5,
            "count": 16,
            "nrounds": 0,
            "ntrophies": -1,
            "name": "Arrows <<--<<",
            "subgroups": [
                {
                    "rankid": 5,
                    "count": 16,
                    "name": "Arrows <<--<<"
                }
            ]
        },
        {
            "classid": 7,
            "count": 0,
            "nrounds": 0,
            "ntrophies": -1,
            "name": "TheLastClass",
            "subgroups": [
                {
                    "rankid": 7,
                    "count": 0,
                    "name": "TheLastClass"
                }
            ]
        }
    ]
}
EOF

curl_getj "action.php?query=class.list" > $CLASS_LIST
diff $GOLDEN $CLASS_LIST || test_fails "class.list fails to match expected response"

curl_postj action.php "action=rank.delete&rankid=$RANKID2" | check_jsuccess

