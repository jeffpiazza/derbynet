#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

# TODO Need a GPRM-schema test

curl_get "action.php?query=photo.next" | expect_one 'main_photo="photo.php/car/file/cropped/Car-1637.jpg/'
curl_get "action.php?query=photo.next" | expect_one 'inset_photo="photo.php/head/file/cropped/head-A.jpg/'
curl_get "action.php?query=photo.next" | expect_one 'next_photo="photo.php/car/file/cropped/Car-1638.jpg/'

curl_get "action.php?query=photo.next&racerid=0" | expect_one 'main_photo="photo.php/car/file/cropped/Car-1637.jpg/'
curl_get "action.php?query=photo.next&racerid=0" | expect_one 'inset_photo="photo.php/head/file/cropped/head-A.jpg/'
curl_get "action.php?query=photo.next&racerid=0" | expect_one 'next_photo="photo.php/car/file/cropped/Car-1638.jpg/'

# Restrictions by classid:
# classid=1 has Car-1637, then Car-1642
# classid=2 has Car-1638, then Car-1643
# classid=3 has Car-1639, then Car-1644
curl_get "action.php?query=photo.next&racerid=0&classids=2" | expect_one 'main_photo="photo.php/car/file/cropped/Car-1638.jpg/'
curl_get "action.php?query=photo.next&racerid=0&classids=2" | expect_one 'next_photo="photo.php/car/file/cropped/Car-1643.jpg/'
curl_get "action.php?query=photo.next&racerid=0&classids=3" | expect_one 'main_photo="photo.php/car/file/cropped/Car-1639.jpg/'
curl_get "action.php?query=photo.next&racerid=0&classids=3" | expect_one 'next_photo="photo.php/car/file/cropped/Car-1644.jpg/'
curl_get "action.php?query=photo.next&racerid=0&classids=2,3" | expect_one 'main_photo="photo.php/car/file/cropped/Car-1638.jpg/'
curl_get "action.php?query=photo.next&racerid=0&classids=2,3" | expect_one 'next_photo="photo.php/car/file/cropped/Car-1639.jpg/'

curl_get "action.php?query=photo.next&racerid=15" | expect_one 'main_photo="photo.php/car/file/cropped/Car-1652.jpg/'
curl_get "action.php?query=photo.next&racerid=15" | expect_one 'inset_photo="photo.php/head/file/cropped/head-E.jpg/'
curl_get "action.php?query=photo.next&racerid=15" | expect_one 'next_photo="photo.php/car/file/cropped/Car-1688.jpg/'

# Racerid 32 (Freddie Font) has car but no head shot
curl_get "action.php?query=photo.next&racerid=31" | expect_one 'main_photo="photo.php/car/file/cropped/Car-1861.jpg/'
curl_get "action.php?query=photo.next&racerid=31" | expect_one 'inset_photo="photo.php/head/file/cropped/Cub-1478.jpg/'
curl_get "action.php?query=photo.next&racerid=31" | expect_one 'next_photo="photo.php/head/file/cropped/Cub-0764.jpg/'

# Racerid 33 (Freeman Fizer) has head shot but no car photo
curl_get "action.php?query=photo.next&racerid=32" | expect_one 'main_photo="photo.php/head/file/cropped/Cub-0764.jpg/'
curl_get "action.php?query=photo.next&racerid=32" | expect_count 'inset_photo=' 0
curl_get "action.php?query=photo.next&racerid=32" | expect_one 'next_photo="photo.php/car/file/cropped/Car-1864.jpg/'

# Racerid 34 (Gregg Grove) has neither; get racer 35 instead
curl_get "action.php?query=photo.next&racerid=33" | expect_one 'racerid="35"'
curl_get "action.php?query=photo.next&racerid=33" | expect_one 'main_photo="photo.php/car/file/cropped/Car-1864.jpg/'
curl_get "action.php?query=photo.next&racerid=33" | expect_one 'inset_photo="photo.php/head/file/cropped/Cub-8464.jpg/'
curl_get "action.php?query=photo.next&racerid=33" | expect_one 'next_photo="photo.php/car/file/cropped/Car-1865.jpg/'

curl_get "action.php?query=photo.next&racerid=81" | expect_one 'main_photo="photo.php/car/file/cropped/Car-5639.jpg/'
curl_get "action.php?query=photo.next&racerid=81" | expect_one 'inset_photo="photo.php/head/file/cropped/Cub-6507.jpg/'
curl_get "action.php?query=photo.next&racerid=81" | expect_count 'next_photo=' 0

# Testing the last racer with a photo
curl_get "action.php?query=photo.next&racerid=82" | expect_count '<racer' 0
