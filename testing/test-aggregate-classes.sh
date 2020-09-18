#! /bin/bash

BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

`dirname $0`/reset-database.sh "$BASE_URL"
`dirname $0`/import-roster.sh "$BASE_URL"

`dirname $0`/outlaw-class.sh "$BASE_URL"

curl_post action.php "action=settings.write&unused-lane-mask=0&n-lanes=4" | check_success

# Check in everyone and race all the rounds
curl_post action.php "action=racer.bulk&who=all&what=checkin&value=1" | check_success

curl_post action.php "action=schedule.generate&roundid=1" | check_success
curl_post action.php "action=schedule.generate&roundid=2" | check_success
curl_post action.php "action=schedule.generate&roundid=3" | check_success
curl_post action.php "action=schedule.generate&roundid=4" | check_success
curl_post action.php "action=schedule.generate&roundid=5" | check_success
curl_post action.php "action=schedule.generate&roundid=6" | check_success

echo Starting run_heat

curl_post action.php "action=heat.select&roundid=1&now_racing=1" | check_success
run_heat	1	1	3.488	3.656	3.36	3.518
run_heat	1	2	3.393	3.273	3.346	3.293
run_heat	1	3	3.339	3.698	3.844	3.465
run_heat	1	4	3.26	3.351	3.441	3.672
run_heat	1	5	3.738	3.392	3.717	3.682
run_heat	1	6	3.116	3.591	3.553	3.8
run_heat	1	7	3.026	3.381	3.464	3.843
run_heat	1	8	3.233	3.127	3.63	3.694
run_heat	1	9	3.845	3.144	3.859	3.09
run_heat	1	10	3.345	3.515	3.708	3.15
run_heat	1	11	3.448	3.127	3.407	3.005
run_heat	1	12	3.429	3.329	3.638	3.074
run_heat	1	13	3.37	3.475	3.232	3.869
run_heat	1	14	3.291	3.422	3.571	3.015
run_heat	1	15	3.759	3.874	3.042	3.261
run_heat	1	16	3.452	3.146	3.592	3.882
run_heat	1	17	3.819	3.065	3.761	3.551  x

curl_post action.php "action=heat.select&roundid=2&now_racing=1" | check_success
run_heat	2	1	3.711	3.108	3.42	3.463
run_heat	2	2	3.397	3.019	3.792	3.081
run_heat	2	3	3.71	3.874	3.474	3.757
run_heat	2	4	3.14	3.717	3.816	3.197
run_heat	2	5	3.104	3.395	3.402	3.868
run_heat	2	6	3.788	3.473	3.666	3.803
run_heat	2	7	3.325	3.485	3.645	3.643
run_heat	2	8	3.878	3.302	3.033	3.393
run_heat	2	9	3.751	3.123	3.489	3.344
run_heat	2	10	3.178	3.648	3.843	3.805
run_heat	2	11	3.647	3.093	3.248	3.845
run_heat	2	12	3.085	3.576	3.474	3.451
run_heat	2	13	3.563	3.702	3.107	3.841
run_heat	2	14	3.488	3.266	3.65	3.687
run_heat	2	15	3.245	3.477	3.846	3.571
run_heat	2	16	3.291	3.811	3.786	3.189
run_heat	2	17	3.642	3.182	3.571	3.311 x

curl_post action.php "action=heat.select&roundid=3&now_racing=1" | check_success
run_heat	3	1	3.848	3.865	3.25	3.017
run_heat	3	2	3.507	3.105	3.551	3.748
run_heat	3	3	3.897	3.681	3.482	3.398
run_heat	3	4	3.522	3.647	3.546	3.141
run_heat	3	5	3.718	3.347	3.464	3.65
run_heat	3	6	3.244	3.847	3.872	3.662
run_heat	3	7	3.067	3.723	3.121	3.448
run_heat	3	8	3.076	3.806	3.049	3.53
run_heat	3	9	3.563	3.675	3.495	3.798
run_heat	3	10	3.846	3.124	3.81	3.638
run_heat	3	11	3.786	3.647	3.124	3.501
run_heat	3	12	3.41	3.361	3.76	3.241
run_heat	3	13	3.291	3.62	3.444	3.307
run_heat	3	14	3.554	3.571	3.629	3.765
run_heat	3	15	3.764	3.636	3.001	3.131
run_heat	3	16	3.535	3.205	3.207	3.249 x

curl_post action.php "action=heat.select&roundid=4&now_racing=1" | check_success
run_heat	4	1	3.756	3.737	3.639	3.892
run_heat	4	2	3.141	3.306	3.742	3.087
run_heat	4	3	3.197	3.532	3.436	3.064
run_heat	4	4	3.364	3.104	3.488	3.682
run_heat	4	5	3.105	3.307	3.168	3.099
run_heat	4	6	3.243	3.427	3.36	3.207
run_heat	4	7	3.591	3.581	3.517	3.733
run_heat	4	8	3.433	3.321	3.165	3.056
run_heat	4	9	3.163	3.613	3.133	3.88
run_heat	4	10	3.714	3.176	3.484	3.696
run_heat	4	11	3.078	3.281	3.362	3.177
run_heat	4	12	3.696	3.129	3.218	3.335
run_heat	4	13	3.68	3.102	3.641	3.382
run_heat	4	14	3.035	3.297	3.107	3.762
run_heat	4	15	3.324	3.861	3.045	3.767
run_heat	4	16	3.499	3.468	3.388	3.281 x

curl_post action.php "action=heat.select&roundid=5&now_racing=1" | check_success
run_heat	5	1	3.315	3.62	3.242	3.455
run_heat	5	2	3.773	3.022	3.638	3.08
run_heat	5	3	3.698	3.597	3.512	3.853
run_heat	5	4	3.05	3.831	3.416	3.148
run_heat	5	5	3.214	3.723	3.201	3.595
run_heat	5	6	3.527	3.377	3.843	3.735
run_heat	5	7	3.12	3.092	3.881	3.789
run_heat	5	8	3.601	3.335	3.705	3.058
run_heat	5	9	3.161	3.394	3.769	3.19
run_heat	5	10	3.205	3.642	3.816	3.629
run_heat	5	11	3.563	3.403	3.396	3.029
run_heat	5	12	3.329	3.067	3.418	3.468
run_heat	5	13	3.61	3.679	3.837	3.388
run_heat	5	14	3.867	3.704	3.841	3.61
run_heat	5	15	3.267	3.404	3.357	3.224
run_heat	5	16	3.646	3.184	3.678	3.263 x

# Outlaw round: these are intentionally much faster
curl_post action.php "action=heat.select&roundid=6&now_racing=1" | check_success
run_heat	6	1	2.488	2.656	2.36	2.518
run_heat	6	2	2.393	2.273	2.346	2.293
run_heat	6	3	2.339	2.698	2.844	2.465
run_heat	6	4	2.26	2.351	2.441	2.672
run_heat	6	5	2.738	2.392	2.717	2.682
run_heat	6	6	2.116	2.591	2.553	2.8
run_heat	6	7	2.026	2.381	2.464	2.843

run_heat	6	8	3.233	3.127	3.63	3.694
run_heat	6	9	3.845	3.144	3.859	3.09
run_heat	6	10	3.345	3.515	3.708	3.15
run_heat	6	11	3.448	3.127	3.407	3.005
run_heat	6	12	3.429	3.329	3.638	3.074
run_heat	6	13	3.37	3.475	3.232	3.869
run_heat	6	14	3.819	3.065	3.761	3.551  x

## Create "Younger" aggregate class of classes 1,2
curl_post action.php "action=class.add&constituent_1=1&constituent_2=1&name=Younger" | check_success

## Create "Older" aggregate class of classes 3,4,5
curl_post action.php "action=class.add&constituent_3=1&constituent_4=1&constituent_5=1&name=Older" | check_success

## Create an aggregate of aggregates, "Scouts", that excludes Outlaw
curl_post action.php "action=class.add&constituent_7=1&constituent_8=1&name=Scouts" | check_success


TMP_STANDINGS=$(mktemp /tmp/standings.XXXXX)
curl_get standings.php > $TMP_STANDINGS
echo $TMP_STANDINGS

test "`xmllint --xpath "//option[@value='st-q7']/text()" $TMP_STANDINGS`" = "Younger" || test_fails Younger option

test "`xmllint --xpath "//div[@class='st-q7']/ancestor::tr/td[3]" $TMP_STANDINGS`" \
     = "<td>Edgardo Easterwood</td><td>Pat Petrone</td><td>Adolfo \"Dolf\" Asher</td><td>Felton Fouche</td><td>Carroll Cybulski</td><td>Ben Bittinger</td><td>Danial Depaolo</td><td>Kelvin Knapp</td><td>Darrell &amp; Darrell Delaughter</td><td>Levi Lahr</td><td>Tracey Trapp</td><td>Lewis Levitsky</td><td>Kelvin Kinman</td><td>Owen O'Leary</td><td>Michal Melendrez</td><td>Angelo Alas</td><td>Willard Woolfolk</td><td>Ian Ives</td><td>Blake Burling</td><td>Freddie Font</td><td>Toby Teed</td><td>Mohamed McGrew</td><td>Raymon Ruffner</td><td>Jesse Jara</td><td>Josh Jose</td><td>Dexter Dawes</td><td>Renaldo Raposo</td><td>Christoper Chauncey</td><td>Herb Halfacre</td><td>Royce Rohman</td><td>Elliot Eastman</td><td>Rodrigo Rencher</td><td>Willard Wile</td><td>Derick Dreier</td>" || test_fails Younger class rankings


test "`xmllint --xpath "//option[@value='st-q8']/text()" $TMP_STANDINGS`" = "Older" || test_fails Older option

test "`xmllint --xpath "//div[@class='st-q8']/ancestor::tr/td[3]" $TMP_STANDINGS`" \
     = "<td>Rex Rosalez</td><td>Derek Dantonio</td><td>Denny Deering</td><td>Nelson No</td><td>Sean Strasburg</td><td>Pete Pinkney</td><td>Julian Jarrard</td><td>Darrin Denny</td><td>Lanny Lavigne</td><td>Travis Toothaker</td><td>Byron Billy</td><td>Freeman Fizer</td><td>Harley Howell</td><td>Judson Joynt</td><td>Jamison Jeffress</td><td>Jackson Juliano</td><td>Norbert Nightingale</td><td>Earnest Evangelista</td><td>Emory Ertel</td><td>Barney Bainter</td><td>Cletus Creager</td><td>Ethan Enye</td><td>Robbie Roush</td><td>Gregg Grove</td><td>Bruce Boissonneault</td><td>Markus Muncy</td><td>Vincent Vinci</td><td>Weston Whigham</td><td>Dorian Dunkle</td><td>Arden Aziz</td><td>Jed Jaquez</td><td>Kris Kaba</td><td>Philip Prum</td><td>Marlon McGray</td><td>Numbers Nish</td><td>Juan Jacobsen</td><td>Reuben Rockhill</td><td>Porter Papke</td><td>Enoch Eccles</td><td>Domingo Doles</td><td>Lyman Liller</td><td>Carey Craney</td><td>Scottie Servais</td><td>Timmy Tomei</td><td>Clark Chesnutt</td><td>Kory Kilgo</td><td>Antoine Akiyama</td><td>Ca&#xF1;umil Calero</td>" || test_fails Older class rankings

test "`xmllint --xpath "//option[@value='st-q9']/text()" $TMP_STANDINGS`" = "Scouts" || test_fails Scouts option

test "`xmllint --xpath "//div[@class='st-q9']/ancestor::tr/td[3]" $TMP_STANDINGS`" \
     = "<td>Rex Rosalez</td><td>Derek Dantonio</td><td>Denny Deering</td><td>Edgardo Easterwood</td><td>Pat Petrone</td><td>Nelson No</td><td>Adolfo \"Dolf\" Asher</td><td>Sean Strasburg</td><td>Pete Pinkney</td><td>Felton Fouche</td><td>Carroll Cybulski</td><td>Julian Jarrard</td><td>Darrin Denny</td><td>Ben Bittinger</td><td>Danial Depaolo</td><td>Kelvin Knapp</td><td>Lanny Lavigne</td><td>Travis Toothaker</td><td>Darrell &amp; Darrell Delaughter</td><td>Levi Lahr</td><td>Tracey Trapp</td><td>Byron Billy</td><td>Freeman Fizer</td><td>Harley Howell</td><td>Lewis Levitsky</td><td>Judson Joynt</td><td>Jamison Jeffress</td><td>Jackson Juliano</td><td>Norbert Nightingale</td><td>Earnest Evangelista</td><td>Emory Ertel</td><td>Kelvin Kinman</td><td>Barney Bainter</td><td>Cletus Creager</td><td>Ethan Enye</td><td>Robbie Roush</td><td>Owen O'Leary</td><td>Gregg Grove</td><td>Bruce Boissonneault</td><td>Markus Muncy</td><td>Vincent Vinci</td><td>Michal Melendrez</td><td>Weston Whigham</td><td>Dorian Dunkle</td><td>Arden Aziz</td><td>Jed Jaquez</td><td>Kris Kaba</td><td>Angelo Alas</td><td>Willard Woolfolk</td><td>Ian Ives</td><td>Philip Prum</td><td>Marlon McGray</td><td>Blake Burling</td><td>Freddie Font</td><td>Numbers Nish</td><td>Toby Teed</td><td>Mohamed McGrew</td><td>Juan Jacobsen</td><td>Reuben Rockhill</td><td>Raymon Ruffner</td><td>Porter Papke</td><td>Jesse Jara</td><td>Enoch Eccles</td><td>Josh Jose</td><td>Dexter Dawes</td><td>Domingo Doles</td><td>Renaldo Raposo</td><td>Lyman Liller</td><td>Christoper Chauncey</td><td>Carey Craney</td><td>Herb Halfacre</td><td>Scottie Servais</td><td>Royce Rohman</td><td>Elliot Eastman</td><td>Timmy Tomei</td><td>Rodrigo Rencher</td><td>Clark Chesnutt</td><td>Kory Kilgo</td><td>Antoine Akiyama</td><td>Ca&#xF1;umil Calero</td><td>Willard Wile</td><td>Derick Dreier</td>" || test_fails Scouts class rankings
