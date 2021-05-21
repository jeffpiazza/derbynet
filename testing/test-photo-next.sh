#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

# TODO Need a GPRM-schema test

curl_getj "action.php?query=json.photo.next" | \
    jq '    (.racer["main-photo"] | test("photo.php/car/file/cropped/Car-1637.jpg/.*"))
        and (.racer["inset-photo"] | test("photo.php/head/file/cropped/head-A.jpg/.*"))
        and (.racer["next-photo"] | test("photo.php/car/file/cropped/Car-1638.jpg/.*"))' | \
    expect_eq true

curl_getj "action.php?query=json.photo.next&racerid=0" | \
    jq '    (.racer["main-photo"] | test("photo.php/car/file/cropped/Car-1637.jpg/.*"))
        and (.racer["inset-photo"] | test("photo.php/head/file/cropped/head-A.jpg/.*"))
        and (.racer["next-photo"] | test("photo.php/car/file/cropped/Car-1638.jpg/.*"))' | \
    expect_eq true

# Restrictions by classid:
# classid=1 has Car-1637, then Car-1642
# classid=2 has Car-1638, then Car-1643
# classid=3 has Car-1639, then Car-1644
curl_getj "action.php?query=json.photo.next&racerid=0&classids=2" | \
    jq '    (.racer["main-photo"] | test("photo.php/car/file/cropped/Car-1638.jpg/.*"))
        and (.racer["next-photo"] | test("photo.php/car/file/cropped/Car-1643.jpg/.*"))' | \
    expect_eq true
    
curl_getj "action.php?query=json.photo.next&racerid=0&classids=3" | \
    jq '    (.racer["main-photo"] | test("photo.php/car/file/cropped/Car-1639.jpg/.*"))
        and (.racer["next-photo"] | test("photo.php/car/file/cropped/Car-1644.jpg/.*"))' | \
    expect_eq true

curl_getj "action.php?query=json.photo.next&racerid=0&classids=2,3" | \
    jq '    (.racer["main-photo"] | test("photo.php/car/file/cropped/Car-1638.jpg/.*"))
        and (.racer["next-photo"] | test("photo.php/car/file/cropped/Car-1639.jpg/.*"))' | \
    expect_eq true

curl_getj "action.php?query=json.photo.next&racerid=15" | \
    jq '    (.racer["main-photo"] | test("photo.php/car/file/cropped/Car-1652.jpg/.*"))
        and (.racer["inset-photo"] | test("photo.php/head/file/cropped/head-E.jpg/.*"))
        and (.racer["next-photo"] | test("photo.php/car/file/cropped/Car-1688.jpg/.*"))' | \
    expect_eq true

# Racerid 32 (Freddie Font) has car but no head shot
curl_getj "action.php?query=json.photo.next&racerid=31" | \
    jq '    (.racer["main-photo"] | test("photo.php/car/file/cropped/Car-1861.jpg/.*"))
        and (.racer["inset-photo"] | test("photo.php/head/file/cropped/Cub-1478.jpg/.*"))
        and (.racer["next-photo"] | test("photo.php/head/file/cropped/Cub-0764.jpg/.*"))' | \
    expect_eq true

# Racerid 33 (Freeman Fizer) has head shot but no car photo
curl_getj "action.php?query=json.photo.next&racerid=32" | \
    jq '    (.racer["main-photo"] | test("photo.php/head/file/cropped/Cub-0764.jpg/.*"))
        and (.racer["inset-photo"]? | not)
        and (.racer["next-photo"] | test("photo.php/car/file/cropped/Car-1864.jpg/.*"))' | \
    expect_eq true

# Racerid 34 (Gregg Grove) has neither; get racer 35 instead
curl_getj "action.php?query=json.photo.next&racerid=33" | \
    jq '    (.racer.racerid == 35)
        and (.racer["main-photo"] | test("photo.php/car/file/cropped/Car-1864.jpg/.*"))
        and (.racer["inset-photo"] | test("photo.php/head/file/cropped/Cub-8464.jpg/.*"))
        and (.racer["next-photo"] | test("photo.php/car/file/cropped/Car-1865.jpg/.*"))' | \
    expect_eq true

curl_getj "action.php?query=json.photo.next&racerid=81" |
    jq '    (.racer["main-photo"] | test("photo.php/car/file/cropped/Car-5639.jpg/.*"))
        and (.racer["inset-photo"] | test("photo.php/head/file/cropped/Cub-6507.jpg/.*"))
        and (.racer["next-photo"]? | not)' | \
    expect_eq true

# Testing the last racer with a photo
curl_getj "action.php?query=json.photo.next&racerid=82" | jq '.racer? | not' | expect_eq true
