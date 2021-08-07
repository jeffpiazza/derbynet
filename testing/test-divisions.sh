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
    jq -e '.divisions | length == 1 and .[0].name == "Default" and (.[0].subdivisions | length == 0)' >/dev/null || test_fails

`dirname $0`/reset-database.sh "$BASE_URL"
`dirname $0`/import-subdivided-roster.sh "$BASE_URL"

curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 3 
        and .[0].name == "Default" and .[0].classid == 1
        and (.[0].subgroups | length == 2)
        and .[0].subgroups[0].name == "Subdiv 1a"
        and .[0].subgroups[0].rankid == 1
        and .[0].subgroups[1].name == "Subdiv 1b"
        and .[0].subgroups[1].rankid == 2
        and .[1].name == "Div 2" and .[1].classid == 2
        and (.[1].subgroups | length == 2)
        and .[1].subgroups[0].name == "Subdiv 2b"
        and .[1].subgroups[0].rankid == 3
        and .[1].subgroups[1].name == "Subdiv 2a"
        and .[1].subgroups[1].rankid == 4
        and .[2].name == "Div 3" and .[2].classid == 3
        and (.[2].subgroups | length == 1)
        and .[2].subgroups[0].name == "Div 3"
        and .[2].subgroups[0].rankid == 5
' >/dev/null || test_fails

# With by-division rule, reordering classes should reorder divisions
curl_postj action.php "action=class.order&classid_1=2&classid_2=1&classid_3=3" | check_jsuccess
curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 3 
        and .[0].name == "Div 2" and .[0].classid == 2
        and (.[0].subgroups | length == 2)
        and .[0].subgroups[0].name == "Subdiv 2b"
        and .[0].subgroups[0].rankid == 3
        and .[0].subgroups[1].name == "Subdiv 2a"
        and .[0].subgroups[1].rankid == 4
        and .[1].name == "Default" and .[1].classid == 1
        and (.[1].subgroups | length == 2)
        and .[1].subgroups[0].name == "Subdiv 1a"
        and .[1].subgroups[0].rankid == 1
        and .[1].subgroups[1].name == "Subdiv 1b"
        and .[1].subgroups[1].rankid == 2
        and .[2].name == "Div 3" and .[2].classid == 3
        and (.[2].subgroups | length == 1)
        and .[2].subgroups[0].name == "Div 3"
        and .[2].subgroups[0].rankid == 5
' >/dev/null || test_fails

curl_getj "action.php?query=poll&values=divisions" | \
    jq -e '.divisions | length == 3 
        and .[0].name == "Div 2" and .[0].divisionid == 2
        and (.[0].subdivisions | length == 2)
        and .[0].subdivisions[0].name == "Subdiv 2b"
        and .[0].subdivisions[0].subdivisionid == 3
        and .[0].subdivisions[1].name == "Subdiv 2a"
        and .[0].subdivisions[1].subdivisionid == 4
        and .[1].name == "Default" and .[1].divisionid == 1
        and (.[1].subdivisions | length == 2)
        and .[1].subdivisions[0].name == "Subdiv 1a"
        and .[1].subdivisions[0].subdivisionid == 1
        and .[1].subdivisions[1].name == "Subdiv 1b"
        and .[1].subdivisions[1].subdivisionid == 2
        and .[2].name == "Div 3" and .[2].divisionid == 3
        and (.[2].subdivisions | length == 1)
        and .[2].subdivisions[0].name == "Div 3"
        and .[2].subdivisions[0].subdivisionid == 5
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

# With one-group rule, reordering subdivisions should reorder divisions
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
        and (.[0].subdivisions | length == 2)
        and .[0].subdivisions[0].name == "Subdiv 1a"
        and .[0].subdivisions[0].subdivisionid == 1
        and .[0].subdivisions[1].name == "Subdiv 1b"
        and .[0].subdivisions[1].subdivisionid == 2
        and .[1].name == "Div 2" and .[1].divisionid == 2
        and (.[1].subdivisions | length == 2)
        and .[1].subdivisions[0].name == "Subdiv 2b"
        and .[1].subdivisions[0].subdivisionid == 3
        and .[1].subdivisions[1].name == "Subdiv 2a"
        and .[1].subdivisions[1].subdivisionid == 4
        and .[2].name == "Div 3" and .[2].divisionid == 3
        and (.[2].subdivisions | length == 1)
        and .[2].subdivisions[0].name == "Div 3"
        and .[2].subdivisions[0].subdivisionid == 5
' >/dev/null || test_fails


curl_postj action.php "action=division.apply-rule&rule=by-division" | check_jsuccess

curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 3 
        and .[0].name == "Default" and (.[0].subgroups | length == 2)
        and .[0].subgroups[0].name == "Subdiv 1a"
        and .[0].subgroups[1].name == "Subdiv 1b"
        and .[1].name == "Div 2" and (.[1].subgroups | length == 2)
        and .[1].subgroups[0].name == "Subdiv 2b"
        and .[1].subgroups[1].name == "Subdiv 2a"
        and .[2].name == "Div 3" and (.[2].subgroups | length == 1)
        and .[2].subgroups[0].name == "Div 3"
' >/dev/null || test_fails

curl_postj action.php "action=division.apply-rule&rule=custom" | check_jsuccess

curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 3 
        and .[0].name == "Default" and (.[0].subgroups | length == 2)
        and .[0].subgroups[0].name == "Subdiv 1a" and .[0].subgroups[0].rankid == 1
        and .[0].subgroups[1].name == "Subdiv 1b" and .[0].subgroups[1].rankid == 2
        and .[1].name == "Div 2" and (.[1].subgroups | length == 2)
        and .[1].subgroups[0].name == "Subdiv 2b" and .[1].subgroups[0].rankid == 3
        and .[1].subgroups[1].name == "Subdiv 2a" and .[1].subgroups[1].rankid == 4
        and .[2].name == "Div 3" and (.[2].subgroups | length == 1)
        and .[2].subgroups[0].name == "Div 3" and .[2].subgroups[0].rankid == 5
' >/dev/null || test_fails

curl_postj action.php "action=division.move&div_field=subdivisionid&div_id=3&group_field=classid&group_id=1" | check_jsuccess

curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 3 
        and .[0].name == "Default" and (.[0].subgroups | length == 3)
        and .[0].subgroups[0].name == "Subdiv 1a" and .[0].subgroups[0].rankid == 1
        and .[0].subgroups[1].name == "Subdiv 1b" and .[0].subgroups[1].rankid == 2
        and .[0].subgroups[2].name == "Subdiv 2b" and .[0].subgroups[2].rankid == 6
        and .[1].name == "Div 2" and (.[1].subgroups | length == 2)
        and .[1].subgroups[0].name == "Subdiv 2b" and .[1].subgroups[0].rankid == 3
        and .[1].subgroups[1].name == "Subdiv 2a" and .[1].subgroups[1].rankid == 4
        and .[2].name == "Div 3" and (.[2].subgroups | length == 1)
        and .[2].subgroups[0].name == "Div 3" and .[2].subgroups[0].rankid == 5
' >/dev/null || test_fails

curl_getj "action.php?query=poll&values=divisions" | \
    jq -e '.divisions | length == 3
        and .[0].name == "Default" and (.[0].subdivisions | length == 2)
        and .[0].subdivisions[0].name == "Subdiv 1a" and .[0].subdivisions[0].subdivisionid == 1
        and .[0].subdivisions[1].name == "Subdiv 1b" and .[0].subdivisions[1].subdivisionid == 2
        and .[1].name == "Div 2" and (.[1].subdivisions | length == 2)
        and .[1].subdivisions[0].name == "Subdiv 2b" and .[1].subdivisions[0].subdivisionid == 3
        and .[1].subdivisions[1].name == "Subdiv 2a" and .[1].subdivisions[1].subdivisionid == 4
        and .[2].name == "Div 3" and (.[2].subdivisions | length == 1)
        and .[2].subdivisions[0].name == "Div 3" and .[2].subdivisions[0].subdivisionid == 5
' >/dev/null || test_fails

curl_postj action.php "action=division.apply-rule&rule=custom&cleanup=1" | check_jsuccess

curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 3 
        and .[0].name == "Default" and (.[0].subgroups | length == 3)
        and .[0].subgroups[0].name == "Subdiv 1a" and .[0].subgroups[0].rankid == 1
        and .[0].subgroups[1].name == "Subdiv 1b" and .[0].subgroups[1].rankid == 2
        and .[0].subgroups[2].name == "Subdiv 2b" and .[0].subgroups[2].rankid == 6
        and .[1].name == "Div 2" and (.[1].subgroups | length == 1)
        and .[1].subgroups[0].name == "Subdiv 2a" and .[1].subgroups[0].rankid == 4
        and .[2].name == "Div 3" and (.[2].subgroups | length == 1)
        and .[2].subgroups[0].name == "Div 3" and .[2].subgroups[0].rankid == 5
' >/dev/null || test_fails

curl_postj action.php "action=division.move&div_field=subdivisionid&div_id=3&group_field=classid&group_id=-1" | check_jsuccess


curl_getj "action.php?query=poll&values=classes" | \
    jq -e '.classes | length == 4
        and .[0].name == "Default" and (.[0].subgroups | length == 3)
        and .[0].subgroups[0].name == "Subdiv 1a" and .[0].subgroups[0].rankid == 1
        and .[0].subgroups[1].name == "Subdiv 1b" and .[0].subgroups[1].rankid == 2
        and .[0].subgroups[2].name == "Subdiv 2b" and .[0].subgroups[2].rankid == 6
        and .[1].name == "Div 2" and (.[1].subgroups | length == 1)
        and .[1].subgroups[0].name == "Subdiv 2a" and .[1].subgroups[0].rankid == 4
        and .[2].name == "Div 3" and (.[2].subgroups | length == 1)
        and .[2].subgroups[0].name == "Div 3" and .[2].subgroups[0].rankid == 5
        and .[3].name == "Subdiv 2b" and (.[3].subgroups | length == 1)
        and .[3].subgroups[0].name == "Subdiv 2b" and .[3].subgroups[0].rankid == 7
' >/dev/null || test_fails
curl_getj "action.php?query=poll&values=divisions" | \
    jq -e '.divisions | length == 3
        and .[0].name == "Default" and (.[0].subdivisions | length == 2)
        and .[0].subdivisions[0].name == "Subdiv 1a" and .[0].subdivisions[0].subdivisionid == 1
        and .[0].subdivisions[1].name == "Subdiv 1b" and .[0].subdivisions[1].subdivisionid == 2
        and .[1].name == "Div 2" and (.[1].subdivisions | length == 2)
        and .[1].subdivisions[0].name == "Subdiv 2a" and .[1].subdivisions[0].subdivisionid == 4
        and .[1].subdivisions[1].name == "Subdiv 2b" and .[1].subdivisions[1].subdivisionid == 3
        and .[2].name == "Div 3" and (.[2].subdivisions | length == 1)
        and .[2].subdivisions[0].name == "Div 3" and .[2].subdivisions[0].subdivisionid == 5
' >/dev/null || test_fails

