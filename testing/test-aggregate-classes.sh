#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

`dirname $0`/reset-database.sh "$BASE_URL"
`dirname $0`/import-roster.sh "$BASE_URL"

`dirname $0`/import-csv-roster.sh  "$BASE_URL" "`dirname $0`/data/outlaw-class.csv"

curl_postj action.php "action=settings.write&do-use-subgroups=1&do-use-subgroups-checkbox" | check_jsuccess
curl_postj action.php "action=rank.edit&rankid=6&name=Siblings" | check_jsuccess
curl_postj action.php "action=rank.add&classid=6&name=Parents" | check_jsuccess
curl_postj action.php "action=rank.add&classid=6&name=Desperados" | check_jsuccess

# Sib 6
# P   7
# Des 8

# Move to parents(rankid=7)
curl_postj action.php "action=racer.edit&rankid=7&racer=92" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=7&racer=90" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=7&racer=86" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=7&racer=84" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=7&racer=88" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=7&racer=85" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=7&racer=93" | check_jsuccess

# Move to Desperado(rankid=8)
curl_postj action.php "action=racer.edit&rankid=8&racer=83" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=8&racer=91" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=8&racer=94" | check_jsuccess
curl_postj action.php "action=racer.edit&rankid=8&racer=87" | check_jsuccess

`dirname $0`/run-scout-heats.sh "$BASE_URL"

curl_postj action.php "action=schedule.generate&roundid=6" | check_jsuccess

# Outlaw round: these are intentionally much faster
curl_postj action.php "action=heat.select&roundid=6&now_racing=1" | check_jsuccess
run_heat 6 1   9902:2.488 9907:2.656 9927:2.360 9922:2.518
run_heat 6 2   9932:2.393 9942:2.273 9962:2.346 9957:2.293
run_heat 6 3   9907:2.339 9912:2.698 9932:2.844 9927:2.465
run_heat 6 4   9937:2.260 9947:2.351 9902:2.441 9962:2.672
run_heat 6 5   9907:2.738 9917:2.392 9937:2.717 9932:2.682
run_heat 6 6   9942:2.116 9952:2.591 9907:2.553 9902:2.800
run_heat 6 7   9912:2.026 9922:2.381 9942:2.464 9937:2.843
run_heat 6 8   9947:3.233 9957:3.127 9907:3.630 9907:3.694
run_heat 6 9   9917:3.845 9927:3.144 9947:3.859 9942:3.090
run_heat 6 10  9952:3.345 9962:3.515 9912:3.708 9907:3.150
run_heat 6 11  9922:3.448 9932:3.127 9952:3.407 9947:3.005
run_heat 6 12  9957:3.429 9902:3.329 9917:3.638 9912:3.074
run_heat 6 13  9927:3.370 9937:3.475 9957:3.232 9952:3.869
run_heat 6 14  9962:3.819 9907:3.065 9922:3.761 9917:3.551 x

# P   Belle Starr(92)
# P   Pearl Hart(90)
# Des Doc Holliday(83)
# Des Butch Cassidy(91)
# Sib Sundance Kid(89)
# P   Sam Bass(86)
# P   Cherokee Bill(84)
# Sib Apache Kid(95)
# P   Fred Waiite(88)
# P   Hoodoo Brown(85)
# Sib Billy Kid(96)
# P   John Wesley Hardin(93)
# Des Jesse James(94)
# Des Zip Wyatt(87)

## Create "Younger" aggregate class of classes 1,2
curl_postj action.php "action=class.add&constituent_1=1&constituent_2=1&name=Younger" | check_jsuccess

## Create "Older" aggregate class of classes 3,4,5
curl_postj action.php "action=class.add&constituent_3=1&constituent_4=1&constituent_5=1&name=Older" | check_jsuccess

## Create an aggregate of aggregates, "Scouts", that excludes Outlaw
curl_postj action.php "action=class.add&constituent_7=1&constituent_8=1&name=Scouts" | check_jsuccess

## Create an aggregate of subgroups, "Firsts and Families", that includes
## rankids 1 (Lions & Tigers), 6 (Siblings) and 7 (Parents)
curl_postj action.php "action=class.add&rankid_1=1&rankid_6=1&rankid_7=1&name=Firsts%20and%20Families" \
    | check_jsuccess

TMP_STANDINGS=$(mktemp /tmp/standings.XXXXX)
curl_get standings.php > $TMP_STANDINGS

test "`xmllint --xpath "//option[@value='st-q7']/text()" $TMP_STANDINGS`" = "Younger" || test_fails Younger option

test "`xmllint --xpath "//div[@class='st-q7']/ancestor::tr/td[3]" $TMP_STANDINGS | tr -d "\n"`" \
     = "<td>Edgardo Easterwood</td><td>Pat Petrone</td><td>Adolfo \"Dolf\" Asher</td><td>Felton Fouche</td><td>Carroll Cybulski</td><td>Ben Bittinger</td><td>Danial Depaolo</td><td>Kelvin Knapp</td><td>Darrell &amp; Darrell Delaughter</td><td>Levi Lahr</td><td>Tracey Trapp</td><td>Lewis Levitsky</td><td>Kelvin Kinman</td><td>Owen O'Leary</td><td>Michal Melendrez</td><td>Angelo Alas</td><td>Willard Woolfolk</td><td>Ian Ives</td><td>Blake Burling</td><td>Freddie Font</td><td>Toby Teed</td><td>Mohamed McGrew</td><td>Raymon Ruffner</td><td>Jesse Jara</td><td>Josh Jose</td><td>Dexter Dawes</td><td>Renaldo Raposo</td><td>Christoper Chauncey</td><td>Herb Halfacre</td><td>Royce Rohman</td><td>Elliot Eastman</td><td>Rodrigo Rencher</td><td>Willard Wile</td><td>Derick Dreier</td>" || test_fails Younger class rankings


test "`xmllint --xpath "//option[@value='st-q8']/text()" $TMP_STANDINGS`" = "Older" || test_fails Older option

test "`xmllint --xpath "//div[@class='st-q8']/ancestor::tr/td[3]" $TMP_STANDINGS | tr -d "\n"`" \
     = "<td>Rex Rosalez</td><td>Derek Dantonio</td><td>Denny Deering</td><td>Nelson No</td><td>Sean Strasburg</td><td>Pete Pinkney</td><td>Julian Jarrard</td><td>Darrin Denny</td><td>Lanny Lavigne</td><td>Travis Toothaker</td><td>Byron Billy</td><td>Freeman Fizer</td><td>Harley Howell</td><td>Judson Joynt</td><td>Jamison Jeffress</td><td>Jackson Juliano</td><td>Norbert Nightingale</td><td>Earnest Evangelista</td><td>Emory Ertel</td><td>Barney Bainter</td><td>Cletus Creager</td><td>Ethan Enye</td><td>Robbie Roush</td><td>Gregg Grove</td><td>Bruce Boissonneault</td><td>Markus Muncy</td><td>Vincent Vinci</td><td>Weston Whigham</td><td>Dorian Dunkle</td><td>Arden Aziz</td><td>Jed Jaquez</td><td>Kris Kaba</td><td>Philip Prum</td><td>Marlon McGray</td><td>Numbers Nish</td><td>Juan Jacobsen</td><td>Reuben Rockhill</td><td>Porter Papke</td><td>Enoch Eccles</td><td>Domingo Doles</td><td>Lyman Liller</td><td>Carey Craney</td><td>Scottie Servais</td><td>Timmy Tomei</td><td>Clark Chesnutt</td><td>Kory Kilgo</td><td>Antoine Akiyama</td><td>Cañumil Calero</td>" || test_fails Older class rankings

test "`xmllint --xpath "//option[@value='st-q9']/text()" $TMP_STANDINGS`" = "Scouts" || test_fails Scouts option

test "`xmllint --xpath "//div[@class='st-q9']/ancestor::tr/td[3]" $TMP_STANDINGS | tr -d "\n"`" \
     = "<td>Rex Rosalez</td><td>Derek Dantonio</td><td>Denny Deering</td><td>Edgardo Easterwood</td><td>Pat Petrone</td><td>Nelson No</td><td>Adolfo \"Dolf\" Asher</td><td>Sean Strasburg</td><td>Pete Pinkney</td><td>Felton Fouche</td><td>Carroll Cybulski</td><td>Julian Jarrard</td><td>Darrin Denny</td><td>Ben Bittinger</td><td>Danial Depaolo</td><td>Kelvin Knapp</td><td>Lanny Lavigne</td><td>Travis Toothaker</td><td>Darrell &amp; Darrell Delaughter</td><td>Levi Lahr</td><td>Tracey Trapp</td><td>Byron Billy</td><td>Freeman Fizer</td><td>Harley Howell</td><td>Lewis Levitsky</td><td>Judson Joynt</td><td>Jamison Jeffress</td><td>Jackson Juliano</td><td>Norbert Nightingale</td><td>Earnest Evangelista</td><td>Emory Ertel</td><td>Kelvin Kinman</td><td>Barney Bainter</td><td>Cletus Creager</td><td>Ethan Enye</td><td>Robbie Roush</td><td>Owen O'Leary</td><td>Gregg Grove</td><td>Bruce Boissonneault</td><td>Markus Muncy</td><td>Vincent Vinci</td><td>Michal Melendrez</td><td>Weston Whigham</td><td>Dorian Dunkle</td><td>Arden Aziz</td><td>Jed Jaquez</td><td>Kris Kaba</td><td>Angelo Alas</td><td>Willard Woolfolk</td><td>Ian Ives</td><td>Philip Prum</td><td>Marlon McGray</td><td>Blake Burling</td><td>Freddie Font</td><td>Numbers Nish</td><td>Toby Teed</td><td>Mohamed McGrew</td><td>Juan Jacobsen</td><td>Reuben Rockhill</td><td>Raymon Ruffner</td><td>Porter Papke</td><td>Jesse Jara</td><td>Enoch Eccles</td><td>Josh Jose</td><td>Dexter Dawes</td><td>Domingo Doles</td><td>Renaldo Raposo</td><td>Lyman Liller</td><td>Christoper Chauncey</td><td>Carey Craney</td><td>Herb Halfacre</td><td>Scottie Servais</td><td>Royce Rohman</td><td>Elliot Eastman</td><td>Timmy Tomei</td><td>Rodrigo Rencher</td><td>Clark Chesnutt</td><td>Kory Kilgo</td><td>Antoine Akiyama</td><td>Cañumil Calero</td><td>Willard Wile</td><td>Derick Dreier</td>" || test_fails Scouts class rankings

test "`xmllint --xpath "//option[@value='st-q10']/text()" $TMP_STANDINGS`" = "Firsts and Families" || test_fails Firsts-and-Families option

test "`xmllint --xpath "//div[@class='st-q10']/ancestor::tr/td[3]" $TMP_STANDINGS | tr -d "\n"`" \
     = "<td>Belle Starr</td><td>Pearl Hart</td><td>Sundance Kid</td><td>Sam Bass</td><td>Cherokee Bill</td><td>Apache Kid</td><td>Fred Waiite</td><td>Hoodoo Brown</td><td>Billy Kid</td><td>John Wesley Hardin</td><td>Edgardo Easterwood</td><td>Adolfo \"Dolf\" Asher</td><td>Felton Fouche</td><td>Carroll Cybulski</td><td>Ben Bittinger</td><td>Danial Depaolo</td><td>Levi Lahr</td><td>Kelvin Kinman</td><td>Owen O'Leary</td><td>Michal Melendrez</td><td>Toby Teed</td><td>Raymon Ruffner</td><td>Jesse Jara</td><td>Herb Halfacre</td><td>Rodrigo Rencher</td><td>Willard Wile</td><td>Derick Dreier</td>" || test_fails Firsts-and-Families class rankings

# Test generating a round from an aggregate class by subgroup
RANK_FINAL=`mktemp`
curl_postj action.php "action=roster.new&classid=10&top=2&bucketed=1" | tee $RANK_FINAL | check_jsuccess
jq -e '.finalists | map(.racerid) | sort == [1,26,89,90,92,95]' $RANK_FINAL >/dev/null || test_fail
