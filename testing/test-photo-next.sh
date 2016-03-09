#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

# TODO Need a GPRM-schema test

[ `curl_get "action.php?query=photo.next" | grep -c 'main_photo="photo.php/car/file/cropped/Car-1637.jpg/'` -eq 1 ] || test_fails
[ `curl_get "action.php?query=photo.next" | grep -c 'inset_photo="photo.php/head/file/cropped/Cub-2106.jpg/'` -eq 1 ] || test_fails
[ `curl_get "action.php?query=photo.next" | grep -c 'next_photo="photo.php/car/file/cropped/Car-1638.jpg/'` -eq 1 ] || test_fails


[ `curl_get "action.php?query=photo.next&racerid=0" | grep -c 'main_photo="photo.php/car/file/cropped/Car-1637.jpg/'` -eq 1 ] || test_fails
[ `curl_get "action.php?query=photo.next&racerid=0" | grep -c 'inset_photo="photo.php/head/file/cropped/Cub-2106.jpg/'` -eq 1 ] || test_fails
[ `curl_get "action.php?query=photo.next&racerid=0" | grep -c 'next_photo="photo.php/car/file/cropped/Car-1638.jpg/'` -eq 1 ] || test_fails

[ `curl_get "action.php?query=photo.next&racerid=15" | grep -c 'main_photo="photo.php/car/file/cropped/Car-1652.jpg/'` -eq 1 ] || test_fails
[ `curl_get "action.php?query=photo.next&racerid=15" | grep -c 'inset_photo="photo.php/head/file/cropped/Cub-2773.jpg/'` -eq 1 ] || test_fails
[ `curl_get "action.php?query=photo.next&racerid=15" | grep -c 'next_photo="photo.php/car/file/cropped/Car-1688.jpg/'` -eq 1 ] || test_fails

# Racer 32 has car but no head shot
[ `curl_get "action.php?query=photo.next&racerid=31" | grep -c 'main_photo="photo.php/car/file/cropped/Car-1861.jpg/'` -eq 1 ] || test_fails
[ `curl_get "action.php?query=photo.next&racerid=31" | grep -c 'inset_photo="photo.php/head/file/cropped/Cub-1478.jpg/'` -eq 1 ] || test_fails
[ `curl_get "action.php?query=photo.next&racerid=31" | grep -c 'next_photo="photo.php/head/file/cropped/Cub-0764.jpg/'` -eq 1 ] || test_fails

# Racer 33 has head shot but no car photo
[ `curl_get "action.php?query=photo.next&racerid=32" | grep -c 'main_photo="photo.php/head/file/cropped/Cub-0764.jpg/'` -eq 1 ] || test_fails
[ `curl_get "action.php?query=photo.next&racerid=32" | grep -c 'inset_photo='` -eq 0 ] || test_fails
[ `curl_get "action.php?query=photo.next&racerid=32" | grep -c 'next_photo="photo.php/car/file/cropped/Car-1864.jpg/'` -eq 1 ] || test_fails

# Racer 34 has neither; get racer 35 instead
[ `curl_get "action.php?query=photo.next&racerid=33" | grep -c 'racerid="35"'` -eq 1 ] || test_fails
[ `curl_get "action.php?query=photo.next&racerid=33" | grep -c 'main_photo="photo.php/car/file/cropped/Car-1864.jpg/'` -eq 1 ] || test_fails
[ `curl_get "action.php?query=photo.next&racerid=33" | grep -c 'inset_photo="photo.php/head/file/cropped/Cub-8464.jpg/'` -eq 1 ] || test_fails
[ `curl_get "action.php?query=photo.next&racerid=33" | grep -c 'next_photo="photo.php/car/file/cropped/Car-1865.jpg/'` -eq 1 ] || test_fails

[ `curl_get "action.php?query=photo.next&racerid=81" | grep -c 'main_photo="photo.php/car/file/cropped/Car-5639.jpg/'` -eq 1 ] || test_fails
[ `curl_get "action.php?query=photo.next&racerid=81" | grep -c 'inset_photo="photo.php/head/file/cropped/Cub-6507.jpg/'` -eq 1 ] || test_fails
[ `curl_get "action.php?query=photo.next&racerid=81" | grep -c 'next_photo='` -eq 0 ] || test_fails

# Testing the last racer with a photo
[ `curl_get "action.php?query=photo.next&racerid=82" | grep -c '<racer'` -eq 0 ] || test_fails
