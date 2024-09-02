#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh


curl_postj action.php "action=database.purge&purge=schedules" | check_jsuccess
curl_postj action.php "action=racer.bulk&who=all&what=checkin&value=1" | check_jsuccess

curl_postj action.php "action=test.setup&slide=slide1.png" | check_jsuccess
curl_postj action.php "action=test.setup&slide=slide2.png" | check_jsuccess
curl_postj action.php "action=test.setup&subdir=alt&slide=alt1.png" | check_jsuccess
curl_postj action.php "action=test.setup&subdir=alt&slide=alt2.png" | check_jsuccess

curl_getj "action.php?query=slide.next" | \
    jq '    (.photo.photo | test("slide.php/title"))' | \
    expect_eq true

curl_getj "action.php?query=slide.next&mode=slide" | \
    jq '    (.photo.photo | test("slide.php/slide1"))' | \
    expect_eq true

curl_getj "action.php?query=slide.next&mode=slide&file=slide1.png" | \
    jq '    (.photo.photo | test("slide.php/slide2"))' | \
    expect_eq true

curl_getj "action.php?query=slide.next&mode=slide&file=slide2.png" | \
    jq '    (.photo.photo | test("photo.php/car/file/cropped/Car-1637.jpg/.*"))
        and (.photo.inset | test("photo.php/head/file/cropped/head-A.jpg/.*"))
        and (.photo.next.mode == "racer")
        and (.photo.next.racerid == 1)' | \
    expect_eq true

curl_getj "action.php?query=slide.next&mode=racer&racerid=0" | \
    jq '    (.photo.photo | test("photo.php/car/file/cropped/Car-1637.jpg/.*"))
        and (.photo.inset | test("photo.php/head/file/cropped/head-A.jpg/.*"))
        and (.photo.next.mode == "racer")
        and (.photo.next.racerid == 1)' | \
    expect_eq true

# Restrictions by classid:
# classid=1 has Car-1637, then Car-1642
# classid=2 has Car-1638, then Car-1643
# classid=3 has Car-1639, then Car-1644
curl_getj "action.php?query=slide.next&mode=racer&racerid=0&classids=2" | \
    jq '    (.photo.photo | test("photo.php/car/file/cropped/Car-1638.jpg/.*"))
        and (.photo.next.mode == "racer")
        and (.photo.next.racerid == 2)' | \
    expect_eq true

curl_getj "action.php?query=slide.next&mode=racer&racerid=0&classids=3" | \
    jq '    (.photo.photo | test("photo.php/car/file/cropped/Car-1639.jpg/.*"))
        and (.photo.next.mode == "racer")
        and (.photo.next.racerid == 3)' | \
    expect_eq true

curl_getj "action.php?query=slide.next&mode=racer&racerid=0&classids=2,3" | \
    jq '    (.photo.photo | test("photo.php/car/file/cropped/Car-1638.jpg/.*"))
        and (.photo.next.mode == "racer")
        and (.photo.next.racerid == 2)' | \
    expect_eq true

curl_getj "action.php?query=slide.next&mode=racer&racerid=15" | \
    jq '    (.photo.photo | test("photo.php/car/file/cropped/Car-1652.jpg/.*"))
        and (.photo.inset | test("photo.php/head/file/cropped/head-E.jpg/.*"))
        and (.photo.next.mode == "racer")
        and (.photo.next.racerid == 16)' | \
    expect_eq true

# Racerid 32 (Freddie Font) has car but no head shot
curl_getj "action.php?query=slide.next&mode=racer&racerid=31" | \
    jq '    (.photo.photo | test("photo.php/car/file/cropped/Car-1861.jpg/.*"))
        and (.photo.inset | test("photo.php/head/file/cropped/Cub-1478.jpg/.*"))
        and (.photo.next.mode == "racer")
        and (.photo.next.racerid == 32)' | \
    expect_eq true

# Racerid 33 (Freeman Fizer) has head shot but no car photo
curl_getj "action.php?query=slide.next&mode=racer&racerid=32" | \
    jq '    (.photo.photo | test("photo.php/head/file/cropped/Cub-0764.jpg/.*"))
        and (.photo.inset? | not)
        and (.photo.next.mode == "racer")
        and (.photo.next.racerid == 33)' | \
    expect_eq true

# Racerid 34 (Gregg Grove) has neither; get racer 35 instead
curl_getj "action.php?query=slide.next&mode=racer&racerid=33" | \
    jq '    (.photo.photo | test("photo.php/car/file/cropped/Car-1864.jpg/.*"))
        and (.photo.inset | test("photo.php/head/file/cropped/Cub-8464.jpg/.*"))
        and (.photo.carnumber == 435)
        and (.photo.name == "Harley Howell")
        and (.photo.next.mode == "racer")
        and (.photo.next.racerid == 35)' | \
    expect_eq true

curl_getj "action.php?query=slide.next&mode=racer&racerid=81" |
    jq '    (.photo.photo | test("photo.php/car/file/cropped/Car-5639.jpg/.*"))
        and (.photo.inset | test("photo.php/head/file/cropped/Cub-6507.jpg/.*"))
        and (.photo.name == "Willard Woolfolk")
        and (.photo.next.mode == "racer")
        and (.photo.next.racerid == 82)' | \
    expect_eq true

curl_getj "action.php?query=slide.next&mode=racer&racerid=82" |
    jq '    (.photo.photo == "slide.php/title")
        and (.photo.title == true)
        and (.photo.next.mode == "slide")
        and (.photo.next.file == "")' | \
    expect_eq true

curl_getj "action.php?query=slide.next&mode=slide&file=zzz" |
    jq '    (.photo.photo | test("photo.php/car/file/cropped/Car-1637.jpg/.*"))
        and (.photo.inset | test("photo.php/head/file/cropped/head-A.jpg/.*"))
        and (.photo.next.mode == "racer")
        and (.photo.next.racerid == 1)' | \
    expect_eq true

# Alt slideshow
curl_getj "action.php?query=slide.next&subdir=alt" | \
    jq '    (.photo.photo | test("slide.php/title"))' | \
    expect_eq true

curl_getj "action.php?query=slide.next&subdir=alt&mode=slide" | \
    jq '    (.photo.photo | test("slide.php/alt/alt1.png"))' | \
    expect_eq true

curl_getj "action.php?query=slide.next&subdir=alt&mode=slide&file=alt1.png" | \
    jq '    (.photo.photo | test("slide.php/alt/alt2.png"))' | \
    expect_eq true

curl_getj "action.php?query=slide.next&subdir=alt&mode=slide&file=alt2.png" | \
    jq '    (.photo.photo | test("photo.php/car/file/cropped/Car-1637.jpg/.*"))
        and (.photo.inset | test("photo.php/head/file/cropped/head-A.jpg/.*"))
        and (.photo.next.mode == "racer")
        and (.photo.next.racerid == 1)' | \
    expect_eq true
