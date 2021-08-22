#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

`dirname $0`/reset-database.sh "$BASE_URL"
curl_postj action.php "action=racer.import&firstname=Kristie&lastname=Kyzer" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Shana&lastname=Sester" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Glendora&lastname=Giusti" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Ophelia&lastname=Oja" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Mirna&lastname=Manier" | check_jsuccess

curl_getj "action.php?query=poll&values=divisions" | \
    jq -e '.divisions | length == 1 and .[0].name == "Default"' >/dev/null || test_fails

`dirname $0`/reset-database.sh "$BASE_URL"
`dirname $0`/import-divided-roster.sh "$BASE_URL"

curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 3 
        and .[0].name == "Default" and .[0].classid == 1
        and (.[0].subgroups | length == 1)
        and .[0].subgroups[0].name == "Default"
        and .[0].subgroups[0].rankid == 1
        and .[1].name == "Div 2" and .[1].classid == 2
        and (.[1].subgroups | length == 1)
        and .[1].subgroups[0].name == "Div 2"
        and .[1].subgroups[0].rankid == 2
        and .[2].name == "Div 3" and .[2].classid == 3
        and (.[2].subgroups | length == 1)
        and .[2].subgroups[0].name == "Div 3"
        and .[2].subgroups[0].rankid == 3
' >/dev/null || test_fails

# With by-division rule, reordering classes should reorder divisions
curl_postj action.php "action=class.order&classid_1=2&classid_2=1&classid_3=3" | check_jsuccess
curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 3 
        and .[0].name == "Div 2" and .[0].classid == 2
        and (.[0].subgroups | length == 1)
        and .[0].subgroups[0].name == "Div 2"
        and .[0].subgroups[0].rankid == 2
        and .[1].name == "Default" and .[1].classid == 1
        and (.[1].subgroups | length == 1)
        and .[1].subgroups[0].name == "Default"
        and .[1].subgroups[0].rankid == 1
        and .[2].name == "Div 3" and .[2].classid == 3
        and (.[2].subgroups | length == 1)
        and .[2].subgroups[0].name == "Div 3"
        and .[2].subgroups[0].rankid == 3
' >/dev/null || test_fails

curl_getj "action.php?query=poll&values=divisions" | \
    jq -e '.divisions | length == 3 
        and .[0].name == "Div 2" and .[0].divisionid == 2
        and .[1].name == "Default" and .[1].divisionid == 1
        and .[2].name == "Div 3" and .[2].divisionid == 3
' >/dev/null || test_fails

curl_postj action.php "action=division.apply-rule&rule=one-group" | check_jsuccess

curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 1
        and .[0].name == "One Group" and (.[0].subgroups | length == 3)
        and .[0].subgroups[0].name == "Div 2"
        and .[0].subgroups[0].rankid == 2
        and .[0].subgroups[1].name == "Default"
        and .[0].subgroups[1].rankid == 1
        and .[0].subgroups[2].name == "Div 3"
        and .[0].subgroups[2].rankid == 3
' >/dev/null || test_fails

# With one-group rule, reordering subdgroups should reorder divisions
curl_postj action.php "action=rank.order&rankid_1=1&rankid_2=2&rankid_3=3" | check_jsuccess
curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 1
        and .[0].name == "One Group" and (.[0].subgroups | length == 3)
        and .[0].subgroups[0].name == "Default"
        and .[0].subgroups[0].rankid == 1
        and .[0].subgroups[1].name == "Div 2"
        and .[0].subgroups[1].rankid == 2
        and .[0].subgroups[2].name == "Div 3"
        and .[0].subgroups[2].rankid == 3
' >/dev/null || test_fails
curl_getj "action.php?query=poll&values=divisions" | \
    jq -e '.divisions | length == 3 
        and .[0].name == "Default" and .[0].divisionid == 1
        and .[1].name == "Div 2" and .[1].divisionid == 2
        and .[2].name == "Div 3" and .[2].divisionid == 3
' >/dev/null || test_fails


curl_postj action.php "action=division.apply-rule&rule=by-division" | check_jsuccess

curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 3 
        and .[0].name == "Default" and (.[0].subgroups | length == 1)
        and .[0].subgroups[0].name == "Default"
        and .[1].name == "Div 2" and (.[1].subgroups | length == 1)
        and .[1].subgroups[0].name == "Div 2"
        and .[2].name == "Div 3" and (.[2].subgroups | length == 1)
        and .[2].subgroups[0].name == "Div 3"
' >/dev/null || test_fails

curl_postj action.php "action=division.apply-rule&rule=custom" | check_jsuccess

curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 3 
        and .[0].name == "Default" and (.[0].subgroups | length == 1)
        and .[0].subgroups[0].name == "Default" and .[0].subgroups[0].rankid == 1
        and .[1].name == "Div 2" and (.[1].subgroups | length == 1)
        and .[1].subgroups[0].name == "Div 2" and .[1].subgroups[0].rankid == 2
        and .[2].name == "Div 3" and (.[2].subgroups | length == 1)
        and .[2].subgroups[0].name == "Div 3" and .[2].subgroups[0].rankid == 3
' >/dev/null || test_fails

curl_postj action.php "action=division.move&div_field=divisionid&div_id=3&group_field=classid&group_id=1" | check_jsuccess

curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 3 
        and .[0].name == "Default" and (.[0].subgroups | length == 2)
        and .[0].subgroups[0].name == "Default" and .[0].subgroups[0].rankid == 1
        and .[0].subgroups[1].name == "Div 3" and .[0].subgroups[1].rankid == 4
        and .[1].name == "Div 2" and (.[1].subgroups | length == 1)
        and .[1].subgroups[0].name == "Div 2" and .[1].subgroups[0].rankid == 2
        and .[2].name == "Div 3" and (.[2].subgroups | length == 1)
        and .[2].subgroups[0].name == "Div 3" and .[2].subgroups[0].rankid == 3
' >/dev/null || test_fails

curl_getj "action.php?query=poll&values=divisions" | \
    jq -e '.divisions | length == 3
        and .[0].name == "Default"
        and .[1].name == "Div 3"
        and .[2].name == "Div 2"
' >/dev/null || test_fails

curl_postj action.php "action=division.apply-rule&rule=custom&cleanup=1" | check_jsuccess

curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 2
        and .[0].name == "Default" and (.[0].subgroups | length == 2)
        and .[0].subgroups[0].name == "Default" and .[0].subgroups[0].rankid == 1
        and .[0].subgroups[1].name == "Div 3" and .[0].subgroups[1].rankid == 4
        and .[1].name == "Div 2" and (.[1].subgroups | length == 1)
        and .[1].subgroups[0].name == "Div 2" and .[1].subgroups[0].rankid == 2
' >/dev/null || test_fails

curl_postj action.php "action=division.move&div_field=divisionid&div_id=3&group_field=classid&group_id=-1" | check_jsuccess

curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 3
        and .[0].name == "Default" and (.[0].subgroups | length == 2)
        and .[0].subgroups[0].name == "Default" and .[0].subgroups[0].rankid == 1
        and .[0].subgroups[1].name == "Div 3" and .[0].subgroups[1].rankid == 4
            and .[0].subgroups[1].count == 0
        and .[1].name == "Div 2" and (.[1].subgroups | length == 1)
        and .[1].subgroups[0].name == "Div 2" and .[1].subgroups[0].rankid == 2
        and .[2].name == "Div 3" and (.[2].subgroups | length == 1)
        and .[2].subgroups[0].name == "Div 3" and .[2].subgroups[0].rankid == 5
            and .[2].subgroups[0].count == 10
' >/dev/null || test_fails
curl_getj "action.php?query=poll&values=divisions" | \
    jq -e '.divisions | length == 3
        and .[0].name == "Default"
        and .[1].name == "Div 2"
        and .[2].name == "Div 3"
' >/dev/null || test_fails

curl_postj action.php "action=division.apply-rule&rule=by-division" | check_jsuccess
curl_getj "action.php?query=poll&values=divisions" | \
    jq -e '.divisions | length == 3
        and .[0].name == "Default"
        and .[1].name == "Div 2"
        and .[2].name == "Div 3"
' >/dev/null || test_fails
curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 3
        and .[0].name == "Default"
        and .[1].name == "Div 2"
        and .[2].name == "Div 3"
' >/dev/null || test_fails

curl_postj action.php "action=division.order&divisionid_1=3&divisionid_2=2&divisionid_3=1" | check_jsuccess
curl_getj "action.php?query=poll&values=divisions" | \
    jq -e '.divisions | length == 3
        and .[0].name == "Div 3" and .[0].divisionid == 3
        and .[1].name == "Div 2" and .[1].divisionid == 2
        and .[2].name == "Default" and .[2].divisionid == 1
' >/dev/null || test_fails
curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 3
        and .[0].name == "Div 3"
        and .[1].name == "Div 2"
        and .[2].name == "Default"
' >/dev/null || test_fails

curl_postj action.php "action=division.apply-rule&rule=one-group" | check_jsuccess
curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 1
        and .[0].name == "One Group"
        and (.[0].subgroups | length == 3)
        and .[0].subgroups[0].name == "Div 3"
        and .[0].subgroups[1].name == "Div 2"
        and .[0].subgroups[2].name == "Default"
' >/dev/null || test_fails
curl_postj action.php "action=division.order&divisionid_1=2&divisionid_2=3&divisionid_3=1" | check_jsuccess
curl_getj "action.php?query=poll&values=divisions" | \
    jq -e '.divisions | length == 3
        and .[0].name == "Div 2" and .[0].divisionid == 2
        and .[1].name == "Div 3" and .[1].divisionid == 3
        and .[2].name == "Default" and .[2].divisionid == 1
' >/dev/null || test_fails
curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 1
        and .[0].name == "One Group"
        and (.[0].subgroups | length == 3)
        and .[0].subgroups[0].name == "Div 2"
        and .[0].subgroups[1].name == "Div 3"
        and .[0].subgroups[2].name == "Default"
' >/dev/null || test_fails

curl_postj action.php "action=division.edit&divisionid=1&name=Div%202" | check_jfailure
curl_postj action.php "action=division.edit&divisionid=1&name=Default" | check_jsuccess # no-op
curl_postj action.php "action=division.edit&divisionid=1&name=Div%201" | check_jsuccess
curl_getj "action.php?query=poll&values=divisions" | \
    jq -e '.divisions | length == 3
        and .[0].name == "Div 2" and .[0].divisionid == 2
        and .[1].name == "Div 3" and .[1].divisionid == 3
        and .[2].name == "Div 1" and .[2].divisionid == 1
' >/dev/null || test_fails
curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 1
        and .[0].name == "One Group"
        and (.[0].subgroups | length == 3)
        and .[0].subgroups[0].name == "Div 2"
        and .[0].subgroups[1].name == "Div 3"
        and .[0].subgroups[2].name == "Div 1"
' >/dev/null || test_fails

curl_postj action.php "action=division.add&name=Div%201" | check_jfailure
curl_postj action.php "action=division.add&name=New%20Div" | check_jsuccess
curl_getj "action.php?query=poll&values=divisions" | \
    jq -e '.divisions | length == 4
        and .[0].name == "Div 2" and .[0].divisionid == 2
        and .[1].name == "Div 3" and .[1].divisionid == 3
        and .[2].name == "Div 1" and .[2].divisionid == 1
        and .[3].name == "New Div" and .[3].divisionid == 4
' >/dev/null || test_fails
curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 1
        and .[0].name == "One Group"
        and (.[0].subgroups | length == 4)
        and .[0].subgroups[0].name == "Div 2"
        and .[0].subgroups[1].name == "Div 3"
        and .[0].subgroups[2].name == "Div 1"
        and .[0].subgroups[3].name == "New Div"
' >/dev/null || test_fails

# Move all the "Div 3" racers to "New Div", then delete "Div 3"
for RACERID in 5 10 15 20 25 30 35 40 45 50; do
    curl_postj action.php "action=racer.edit&racerid=$RACERID&divisionid=4" | check_jsuccess
done
curl_postj action.php "action=division.delete&divisionid=3" | check_jsuccess


