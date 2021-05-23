#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

curl_postj action.php "action=racer.import&firstname=Doc&lastname=Holliday&classname=Outlaw&carnumber=9902" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Cherokee&lastname=Bill&classname=Outlaw&carnumber=9907" | check_jsuccess
curl_postj action.php "action=racer.import&first-last=Hoodoo Brown&classname=Outlaw&carnumber=9907" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Sam&lastname=Bass&classname=Outlaw&carnumber=9912" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Zip&lastname=Wyatt&classname=Outlaw&carnumber=9917" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Fred&lastname=Waiite&classname=Outlaw&carnumber=9922" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Sundance&lastname=Kid&classname=Outlaw&carnumber=9927" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Pearl&lastname=Hart&classname=Outlaw&carnumber=9932" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Butch&lastname=Cassidy&classname=Outlaw&carnumber=9937" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Belle&lastname=Starr&classname=Outlaw&carnumber=9942" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=John%20Wesley&lastname=Hardin&classname=Outlaw&carnumber=9947" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Jesse&lastname=James&classname=Outlaw&carnumber=9952" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Apache&lastname=Kid&classname=Outlaw&carnumber=9957" | check_jsuccess
curl_postj action.php "action=racer.import&firstname=Billy&lastname=Kid&classname=Outlaw&carnumber=9962" | check_jsuccess

