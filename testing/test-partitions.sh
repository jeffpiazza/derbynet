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

curl_getj "action.php?query=poll&values=partitions" | \
    jq -e '.partitions | length == 1 and .[0].name == "Default"' >/dev/null || test_fails

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

# With by-partition rule, reordering classes should reorder partitions
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

curl_getj "action.php?query=poll&values=partitions" | \
    jq -e '.partitions | length == 3 
        and .[0].name == "Div 2" and .[0].partitionid == 2
        and .[1].name == "Default" and .[1].partitionid == 1
        and .[2].name == "Div 3" and .[2].partitionid == 3
' >/dev/null || test_fails

curl_postj action.php "action=partition.apply-rule&rule=one-group" | check_jsuccess

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

# With one-group rule, reordering subdgroups should reorder partitions
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
curl_getj "action.php?query=poll&values=partitions" | \
    jq -e '.partitions | length == 3 
        and .[0].name == "Default" and .[0].partitionid == 1
        and .[1].name == "Div 2" and .[1].partitionid == 2
        and .[2].name == "Div 3" and .[2].partitionid == 3
' >/dev/null || test_fails


curl_postj action.php "action=partition.apply-rule&rule=by-partition" | check_jsuccess

curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 3 
        and .[0].name == "Default" and (.[0].subgroups | length == 1)
        and .[0].subgroups[0].name == "Default"
        and .[1].name == "Div 2" and (.[1].subgroups | length == 1)
        and .[1].subgroups[0].name == "Div 2"
        and .[2].name == "Div 3" and (.[2].subgroups | length == 1)
        and .[2].subgroups[0].name == "Div 3"
' >/dev/null || test_fails

curl_postj action.php "action=partition.apply-rule&rule=custom" | check_jsuccess

curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 3 
        and .[0].name == "Default" and (.[0].subgroups | length == 1)
        and .[0].subgroups[0].name == "Default" and .[0].subgroups[0].rankid == 1
        and .[1].name == "Div 2" and (.[1].subgroups | length == 1)
        and .[1].subgroups[0].name == "Div 2" and .[1].subgroups[0].rankid == 2
        and .[2].name == "Div 3" and (.[2].subgroups | length == 1)
        and .[2].subgroups[0].name == "Div 3" and .[2].subgroups[0].rankid == 3
' >/dev/null || test_fails

curl_postj action.php "action=partition.move&div_field=partitionid&div_id=3&group_field=classid&group_id=1" | check_jsuccess

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

curl_getj "action.php?query=poll&values=partitions" | \
    jq -e '.partitions | length == 3
        and .[0].name == "Default"
        and .[1].name == "Div 3"
        and .[2].name == "Div 2"
' >/dev/null || test_fails

curl_postj action.php "action=partition.apply-rule&rule=custom&cleanup=1" | check_jsuccess

curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 2
        and .[0].name == "Default" and (.[0].subgroups | length == 2)
        and .[0].subgroups[0].name == "Default" and .[0].subgroups[0].rankid == 1
        and .[0].subgroups[1].name == "Div 3" and .[0].subgroups[1].rankid == 4
        and .[1].name == "Div 2" and (.[1].subgroups | length == 1)
        and .[1].subgroups[0].name == "Div 2" and .[1].subgroups[0].rankid == 2
' >/dev/null || test_fails

curl_postj action.php "action=partition.move&div_field=partitionid&div_id=3&group_field=classid&group_id=-1" | check_jsuccess

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
curl_getj "action.php?query=poll&values=partitions" | \
    jq -e '.partitions | length == 3
        and .[0].name == "Default"
        and .[1].name == "Div 2"
        and .[2].name == "Div 3"
' >/dev/null || test_fails

curl_postj action.php "action=partition.apply-rule&rule=by-partition" | check_jsuccess
curl_getj "action.php?query=poll&values=partitions" | \
    jq -e '.partitions | length == 3
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

curl_postj action.php "action=partition.order&partitionid_1=3&partitionid_2=2&partitionid_3=1" | check_jsuccess
curl_getj "action.php?query=poll&values=partitions" | \
    jq -e '.partitions | length == 3
        and .[0].name == "Div 3" and .[0].partitionid == 3
        and .[1].name == "Div 2" and .[1].partitionid == 2
        and .[2].name == "Default" and .[2].partitionid == 1
' >/dev/null || test_fails
curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 3
        and .[0].name == "Div 3"
        and .[1].name == "Div 2"
        and .[2].name == "Default"
' >/dev/null || test_fails

curl_postj action.php "action=partition.apply-rule&rule=one-group" | check_jsuccess
curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 1
        and .[0].name == "One Group"
        and (.[0].subgroups | length == 3)
        and .[0].subgroups[0].name == "Div 3"
        and .[0].subgroups[1].name == "Div 2"
        and .[0].subgroups[2].name == "Default"
' >/dev/null || test_fails
curl_postj action.php "action=partition.order&partitionid_1=2&partitionid_2=3&partitionid_3=1" | check_jsuccess
curl_getj "action.php?query=poll&values=partitions" | \
    jq -e '.partitions | length == 3
        and .[0].name == "Div 2" and .[0].partitionid == 2
        and .[1].name == "Div 3" and .[1].partitionid == 3
        and .[2].name == "Default" and .[2].partitionid == 1
' >/dev/null || test_fails
curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 1
        and .[0].name == "One Group"
        and (.[0].subgroups | length == 3)
        and .[0].subgroups[0].name == "Div 2"
        and .[0].subgroups[1].name == "Div 3"
        and .[0].subgroups[2].name == "Default"
' >/dev/null || test_fails

curl_postj action.php "action=partition.edit&partitionid=1&name=Div%202" | check_jfailure
curl_postj action.php "action=partition.edit&partitionid=1&name=Default" | check_jsuccess # no-op
curl_postj action.php "action=partition.edit&partitionid=1&name=Div%201" | check_jsuccess
curl_getj "action.php?query=poll&values=partitions" | \
    jq -e '.partitions | length == 3
        and .[0].name == "Div 2" and .[0].partitionid == 2
        and .[1].name == "Div 3" and .[1].partitionid == 3
        and .[2].name == "Div 1" and .[2].partitionid == 1
' >/dev/null || test_fails
curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 1
        and .[0].name == "One Group"
        and (.[0].subgroups | length == 3)
        and .[0].subgroups[0].name == "Div 2"
        and .[0].subgroups[1].name == "Div 3"
        and .[0].subgroups[2].name == "Div 1"
' >/dev/null || test_fails

curl_postj action.php "action=partition.add&name=Div%201" | check_jfailure
curl_postj action.php "action=partition.add&name=New%20Div" | check_jsuccess
curl_getj "action.php?query=poll&values=partitions" | \
    jq -e '.partitions | length == 4
        and .[0].name == "Div 2" and .[0].partitionid == 2
        and .[1].name == "Div 3" and .[1].partitionid == 3
        and .[2].name == "Div 1" and .[2].partitionid == 1
        and .[3].name == "New Div" and .[3].partitionid == 4
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
    curl_postj action.php "action=racer.edit&racerid=$RACERID&partitionid=4" | check_jsuccess
done
curl_postj action.php "action=partition.delete&partitionid=3" | check_jsuccess


