#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

# TODO Need a GPRM-schema test

curl_getj "action.php?query=photo.next" | \
    jq '    (.photo.photo | test("slide.php/title"))' | \
    expect_eq true

curl_getj "action.php?query=photo.next&mode=racer&racerid=0" | \
    jq '    (.photo.photo | test("photo.php/car/file/cropped/Car-1637.jpg/.*"))
        and (.photo.inset | test("photo.php/head/file/cropped/head-A.jpg/.*"))
        and (.photo.next.mode == "racer")
        and (.photo.next.racerid == 1)' | \
    expect_eq true

# Restrictions by classid:
# classid=1 has Car-1637, then Car-1642
# classid=2 has Car-1638, then Car-1643
# classid=3 has Car-1639, then Car-1644
curl_getj "action.php?query=photo.next&mode=racer&racerid=0&classids=2" | \
    jq '    (.photo.photo | test("photo.php/car/file/cropped/Car-1638.jpg/.*"))
        and (.photo.next.mode == "racer")
        and (.photo.next.racerid == 2)' | \
    expect_eq true

curl_getj "action.php?query=photo.next&mode=racer&racerid=0&classids=3" | \
    jq '    (.photo.photo | test("photo.php/car/file/cropped/Car-1639.jpg/.*"))
        and (.photo.next.mode == "racer")
        and (.photo.next.racerid == 3)' | \
    expect_eq true

curl_getj "action.php?query=photo.next&mode=racer&racerid=0&classids=2,3" | \
    jq '    (.photo.photo | test("photo.php/car/file/cropped/Car-1638.jpg/.*"))
        and (.photo.next.mode == "racer")
        and (.photo.next.racerid == 2)' | \
    expect_eq true

curl_getj "action.php?query=photo.next&mode=racer&racerid=15" | \
    jq '    (.photo.photo | test("photo.php/car/file/cropped/Car-1652.jpg/.*"))
        and (.photo.inset | test("photo.php/head/file/cropped/head-E.jpg/.*"))
        and (.photo.next.mode == "racer")
        and (.photo.next.racerid == 16)' | \
    expect_eq true

# Racerid 32 (Freddie Font) has car but no head shot
curl_getj "action.php?query=photo.next&mode=racer&racerid=31" | \
    jq '    (.photo.photo | test("photo.php/car/file/cropped/Car-1861.jpg/.*"))
        and (.photo.inset | test("photo.php/head/file/cropped/Cub-1478.jpg/.*"))
        and (.photo.next.mode == "racer")
        and (.photo.next.racerid == 32)' | \
    expect_eq true

# Racerid 33 (Freeman Fizer) has head shot but no car photo
curl_getj "action.php?query=photo.next&mode=racer&racerid=32" | \
    jq '    (.photo.photo | test("photo.php/head/file/cropped/Cub-0764.jpg/.*"))
        and (.photo.inset? | not)
        and (.photo.next.mode == "racer")
        and (.photo.next.racerid == 33)' | \
    expect_eq true

# Racerid 34 (Gregg Grove) has neither; get racer 35 instead
curl_getj "action.php?query=photo.next&mode=racer&racerid=33" | \
    jq '    (.photo.photo | test("photo.php/car/file/cropped/Car-1864.jpg/.*"))
        and (.photo.inset | test("photo.php/head/file/cropped/Cub-8464.jpg/.*"))
        and (.photo.carnumber == 435)
        and (.photo.name == "Harley Howell")
        and (.photo.next.mode == "racer")
        and (.photo.next.racerid == 35)' | \
    expect_eq true

curl_getj "action.php?query=photo.next&mode=racer&racerid=81" |
    jq '    (.photo.photo | test("photo.php/car/file/cropped/Car-5639.jpg/.*"))
        and (.photo.inset | test("photo.php/head/file/cropped/Cub-6507.jpg/.*"))
        and (.photo.name == "Willard Woolfolk")
        and (.photo.next.mode == "racer")
        and (.photo.next.racerid == 82)' | \
    expect_eq true

curl_getj "action.php?query=photo.next&mode=racer&racerid=82" |
    jq '    (.photo.photo == "slide.php/title")
        and (.photo.title == true)
        and (.photo.next.mode == "slide")
        and (.photo.next.file == "")' | \
    expect_eq true

curl_getj "action.php?query=photo.next&mode=slide&file=zzz" |
    jq '    (.photo.photo | test("photo.php/car/file/cropped/Car-1637.jpg/.*"))
        and (.photo.inset | test("photo.php/head/file/cropped/head-A.jpg/.*"))
        and (.photo.next.mode == "racer")
        and (.photo.next.racerid == 1)' | \
    expect_eq true
