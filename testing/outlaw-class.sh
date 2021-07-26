#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

curl_postj action.php "action=racer.import&firstname=Doc&lastname=Holliday&division=Outlaw&carnumber=9902" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Cherokee&lastname=Bill&division=Outlaw&carnumber=9907" | check_jsuccess
curl_postj action.php "action=racer.import&first-last=Hoodoo Brown&division=Outlaw&carnumber=9907" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Sam&lastname=Bass&division=Outlaw&carnumber=9912" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Zip&lastname=Wyatt&division=Outlaw&carnumber=9917" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Fred&lastname=Waiite&division=Outlaw&carnumber=9922" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Sundance&lastname=Kid&division=Outlaw&carnumber=9927" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Pearl&lastname=Hart&division=Outlaw&carnumber=9932" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Butch&lastname=Cassidy&division=Outlaw&carnumber=9937" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Belle&lastname=Starr&division=Outlaw&carnumber=9942" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=John%20Wesley&lastname=Hardin&division=Outlaw&carnumber=9947" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Jesse&lastname=James&division=Outlaw&carnumber=9952" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Apache&lastname=Kid&division=Outlaw&carnumber=9957" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Billy&lastname=Kid&division=Outlaw&carnumber=9962" | check_jsuccess

