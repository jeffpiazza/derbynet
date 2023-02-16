#! /bin/bash


BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

user_login_coordinator

`dirname $0`/reset-database.sh "$BASE_URL"
`dirname $0`/import-roster.sh "$BASE_URL"
### Check in every other racer...
`dirname $0`/test-basic-checkins.sh "$BASE_URL"

# Reinstate car 111
curl_postj action.php "action=racer.edit&racer=11&firstname=Carroll&lastname=Cybulski&carno=111&carname=Vroom&rankid=1&exclude=0" | check_jsuccess

curl_postj action.php "action=settings.write&use-points=1&use-points-checkbox" | check_jsuccess
curl_postj action.php "action=settings.write&n-lanes=4" | check_jsuccess


### Schedule roundid 1
curl_postj action.php "action=schedule.generate&roundid=1" | check_jsuccess
# Racing for roundid=1: 5 heats
curl_postj action.php "action=heat.select&roundid=1&now_racing=1" | check_jsuccess

user_login_timer
curl_post action.php "action=timer-message&message=HELLO" | check_success
curl_post action.php "action=timer-message&message=IDENTIFIED&nlanes=4" | check_success

run_heat -place 1 1   101:2 121:3 141:4 111:1
run_heat -place 1 2   111:1 131:4 101:2 121:3

# Report times for this heat, let timer-message compute places
# First we have a tie for last place in the heat, as for a couple of DNFs;
# both racers get 1 point each (tie for 3rd place)
run_heat 1 3 121:9.9999 141:3.2 111:3.1 131:9.9999

user_login_coordinator
# Check the points for each racer in the heat: # heats, total points
curl_text "standings.php" | grep 121 | expect_one "<td>3</td><td>6</td>"
curl_text "standings.php" | grep 141 | expect_one "<td>2</td><td>4</td>"
curl_text "standings.php" | grep 111 | expect_one "<td>3</td><td>12</td>"
curl_text "standings.php" | grep 131 | expect_one "<td>2</td><td>3</td>"
curl_postj action.php "action=heat.rerun&heat=last" | check_jsuccess
user_login_timer

# Now re-run with a tie for first place
run_heat 1 3  121:3.3 141:3.1 111:3.1 131:3.4

user_login_coordinator
curl_text "standings.php" | grep 121 | expect_one "<td>3</td><td>6</td>"
curl_text "standings.php" | grep 141 | expect_one "<td>2</td><td>5</td>"
curl_text "standings.php" | grep 111 | expect_one "<td>3</td><td>12</td>"
curl_text "standings.php" | grep 131 | expect_one "<td>2</td><td>2</td>"
curl_postj action.php "action=heat.rerun&heat=last" | check_jsuccess
user_login_timer

# Finally, re-run the heat again, now with four good times:
run_heat 1 3  121:3.3 141:3.2 111:3.1 131:3.4

user_login_coordinator
curl_text "standings.php" | grep 121 | expect_one "<td>3</td><td>6</td>"
curl_text "standings.php" | grep 141 | expect_one "<td>2</td><td>4</td>"
curl_text "standings.php" | grep 111 | expect_one "<td>3</td><td>12</td>"
curl_text "standings.php" | grep 131 | expect_one "<td>2</td><td>2</td>"
user_login_timer

run_heat -place 1 4   131:4 101:1 121:2 141:3

user_login_coordinator
curl_getj "action.php?query=poll.coordinator" | jq '.["last-heat"] == "available"' | expect_eq true
curl_postj action.php "action=heat.rerun&heat=last" | check_jsuccess
curl_getj "action.php?query=poll.coordinator" | \
    jq '.["last-heat"] == "recoverable" and 
        (.["heat-results"] | all(has("finishtime") and has("finishplace")))' | \
    expect_eq true

curl_postj action.php "action=heat.reinstate" | grep 'last[_-]heat' | expect_one none
curl_getj "action.php?query=poll.coordinator" | \
    jq '.racers | 
        all((.finishplace==1 and (.name | test("Adolfo.*"))) or 
            (.finishplace==2 and .name == "Derick Dreier") or 
            (.finishplace==3 and .name == "Jesse Jara") or 
            (.finishplace==4 and .name == "Felton Fouche"))' | \
    expect_eq true

curl_postj action.php "action=heat.select&heat=next&now_racing=1" | check_jsuccess
user_login_timer


run_heat -place 1 5   141:2 111:1 131:4 101:3  x

# Results (place):
# 111 101 121 141
# 111 101 121 131
# 111 141 121 131
# 101 121 141 131
# 111 141 101 131
#
#  car           first         last      sum(finishplace)  points
# 111         Carroll        Cybulski     4                16
# 101         Adolfo "Dolf"  Asher        8                12
# 121         Derick         Dreier      11                 9
# 141         Jesse          Jara        11                 9
# 131         Felton         Fouche      16                 4

user_login_coordinator
curl_postj action.php "action=award.present&key=speed-2-1" | check_jsuccess
curl_getj "action.php?query=award.current" | expect_one Asher

curl_getj "action.php?query=poll&values=ondeck" | \
    jq -r '.ondeck.chart | map(select(.resultid == 1))[0].result' | expect_eq z2nd
curl_getj "action.php?query=poll&values=ondeck" | \
    jq -r '.ondeck.chart | map(select(.resultid == 4))[0].result' | expect_eq z1st

# Test the tie in the standings:
#
# These are a little fragile in that they depend on each table row comprising
# exactly one line of text
curl_text "standings.php" | grep Carroll | expect_one "<td class=.col-insuper.>1</td>"
curl_text "standings.php" | grep Asher   | expect_one "<td class=.col-insuper.>2</td>"
curl_text "standings.php" | grep Derick  | expect_one "<td class=.col-insuper.>T3</td>"
curl_text "standings.php" | grep Jesse   | expect_one "<td class=.col-insuper.>T3</td>"
curl_text "standings.php" | grep Felton  | expect_one "<td class=.col-insuper.>5</td>"


curl_text "export.php" | sed -n -e '/START_JSON/,/END_JSON/ p' | tail -2 | head -1 | \
    expect_one '[1,"111","Carroll Cybulski","Vroom","Lions \u0026 Tigers",1,"4","16","1st","1st"]'
curl_text "export.php" | sed -n -e '/START_JSON/,/END_JSON/ p' | tail -2 | head -1 | \
    expect_one '[2,"101","Adolfo \"Dolf\" Asher","","Lions \u0026 Tigers",2,"4","12","1st","3rd"]'
curl_text "export.php" | sed -n -e '/START_JSON/,/END_JSON/ p' | tail -2 | head -1 | \
    expect_one '["T3","121","Derick Dreier","","Lions \u0026 Tigers","T3","4","9","2nd","3rd"]'
curl_text "export.php" | sed -n -e '/START_JSON/,/END_JSON/ p' | tail -2 | head -1 | \
    expect_one '["T3","141","Jesse Jara","","Lions \u0026 Tigers","T3","4","9","2nd","4th"]'
curl_text "export.php" | sed -n -e '/START_JSON/,/END_JSON/ p' | tail -2 | head -1 | \
    expect_one '[5,"131","Felton Fouche","","Lions \u0026 Tigers",5,"4","4","4th","4th"]'

# award presentation when there's a tie for 3rd
curl_postj action.php "action=award.present&key=speed-3a-1" | check_jsuccess
curl_getj "action.php?query=award.current" | expect_one Derick
curl_postj action.php "action=award.present&key=speed-3b-1" | check_jsuccess
curl_getj "action.php?query=award.current" | expect_one Jesse

# There are only two tied for 3rd place, not a third
curl_postj action.php "action=award.present&key=speed-3c-1" | check_jsuccess
curl_getj "action.php?query=award.current" | jq 'length' | expect_eq 0

# There's no fourth place when there's a tie for 3rd
curl_postj action.php "action=award.present&key=speed-4-1" | check_jsuccess
curl_getj "action.php?query=award.current" | jq 'length' | expect_eq 0

# One-trophy-per-racer means Felton takes 1st place in Lions
curl_postj action.php "action=settings.write&one-trophy-per=1&one-trophy-per-checkbox" | check_jsuccess
curl_postj action.php "action=award.present&key=speed-3a" | check_jsuccess
curl_getj "action.php?query=award.current" | expect_one Derick
curl_postj action.php "action=award.present&key=speed-3b" | check_jsuccess
curl_getj "action.php?query=award.current" | expect_one Jesse
curl_postj action.php "action=award.present&key=speed-3c" | check_jsuccess
curl_getj "action.php?query=award.current" | jq 'length' | expect_eq 0
curl_postj action.php "action=award.present&key=speed-4" | check_jsuccess
curl_getj "action.php?query=award.current" | jq 'length' | expect_eq 0
curl_postj action.php "action=award.present&key=speed-1-1" | check_jsuccess
curl_getj "action.php?query=award.current" | expect_one Felton
curl_postj action.php "action=award.present&key=speed-2-1" | check_jsuccess
curl_getj "action.php?query=award.current" | jq 'length' | expect_eq 0

# Generate a next round of top 3, where there's a tie for third -- take 4 finalists
curl_postj action.php "action=roster.new&roundid=1&top=3" | check_jsuccess
jq -e '.finalists | map(.racerid) | sort == [1,11,21,41]' $DEBUG_CURL >/dev/null || \
    test_fails Expecting 4 finalists

curl_postj action.php "action=schedule.generate&roundid=6" | check_jsuccess
# Racing for roundid=6: 4 heats
curl_postj action.php "action=heat.select&roundid=6&now_racing=1" | check_jsuccess

run_heat -place 6 1   111:2 141:3 121:4 101:1
run_heat -place 6 2   101:3 111:4 141:1 121:2
run_heat -place 6 3   121:4 101:1 111:2 141:3
run_heat -place 6 4   141:1 121:2 101:3 111:4  x



# Usage: curl_text standings.php | for_roundid 1 | ...
function for_roundid() {
    grep "<tr.* data-roundid=.$1. "
}

ROUND1_TMP=`mktemp`

curl_text "standings.php" | for_roundid 1 > $ROUND1_TMP
cat $ROUND1_TMP | expect_count '<tr ' 5

cat $ROUND1_TMP | expect_one Carroll
cat $ROUND1_TMP | grep Carroll | expect_one "<div class=.inround.>1</div>"
cat $ROUND1_TMP | grep Carroll | expect_one "<td class=.col-insuper.></td>"

cat $ROUND1_TMP | expect_one Asher
cat $ROUND1_TMP | grep Asher | expect_one "<div class=.inround.>2</div>"
cat $ROUND1_TMP | grep Asher | expect_one "<td class=.col-insuper.></td>"

cat $ROUND1_TMP | expect_one Derick
cat $ROUND1_TMP | grep Derick | expect_one "<div class=.inround.>T3</div>"
cat $ROUND1_TMP | grep Derick | expect_one "<td class=.col-insuper.></td>"

cat $ROUND1_TMP | expect_one Jesse
cat $ROUND1_TMP | grep Jesse | expect_one "<div class=.inround.>T3</div>"
cat $ROUND1_TMP | grep Jesse | expect_one "<td class=.col-insuper.></td>"

cat $ROUND1_TMP | expect_one Felton
cat $ROUND1_TMP | grep Felton | expect_one "<div class=.inround.>5</div>"
cat $ROUND1_TMP | grep Felton | expect_one "<td class=.col-insuper.></td>"

rm $ROUND1_TMP

curl_text "export.php" | sed -n -e '/START_JSON/,/END_JSON/ p' | tail -2 | head -1 | \
    expect_one '[1,"111","Carroll Cybulski","Vroom","Lions \u0026 Tigers",1,1,"4","16","1st","1st"]'
curl_text "export.php" | sed -n -e '/START_JSON/,/END_JSON/ p' | tail -2 | head -1 | \
    expect_one '[2,"101","Adolfo \"Dolf\" Asher","","Lions \u0026 Tigers",2,2,"4","12","1st","3rd"]'
curl_text "export.php" | sed -n -e '/START_JSON/,/END_JSON/ p' | tail -2 | head -1 | \
    expect_one '["T3","121","Derick Dreier","","Lions \u0026 Tigers","T3","T3","4","9","2nd","3rd"]'
curl_text "export.php" | sed -n -e '/START_JSON/,/END_JSON/ p' | tail -2 | head -1 | \
    expect_one '["T3","141","Jesse Jara","","Lions \u0026 Tigers","T3","T3","4","9","2nd","4th"]'
curl_text "export.php" | sed -n -e '/START_JSON/,/END_JSON/ p' | tail -2 | head -1 | \
    expect_one '[5,"131","Felton Fouche","","Lions \u0026 Tigers",5,5,"4","4","4th","4th"]'

ROUND6_TMP=`mktemp`
curl_text "standings.php" | for_roundid 6 > $ROUND6_TMP

cat $ROUND6_TMP | expect_count '<tr ' 4

cat $ROUND6_TMP | expect_one Asher
cat $ROUND6_TMP | grep Asher | expect_one "<div class=.inround.>T1</div>"
cat $ROUND6_TMP | grep Asher | expect_one "<td class=.col-ingroup.>T1</td>"
cat $ROUND6_TMP | grep Asher | expect_one "<div class=.insuper.>T1</div>"
cat $ROUND6_TMP | grep Asher | expect_one "<td class=.col-insuper.>T1</td>"

cat $ROUND6_TMP | expect_one Jesse
cat $ROUND6_TMP | grep Jesse | expect_one "<div class=.inround.>T1</div>"
cat $ROUND6_TMP | grep Jesse | expect_one "<td class=.col-ingroup.>T1</td>"
cat $ROUND6_TMP | grep Jesse | expect_one "<div class=.insuper.>T1</div>"
cat $ROUND6_TMP | grep Jesse | expect_one "<td class=.col-insuper.>T1</td>"

cat $ROUND6_TMP | expect_one Carroll
cat $ROUND6_TMP | grep Carroll | expect_one "<div class=.inround.>T3</div>"
cat $ROUND6_TMP | grep Carroll | expect_one "<td class=.col-ingroup.>T3</td>"
cat $ROUND6_TMP | grep Carroll | expect_one "<div class=.insuper.>T3</div>"
cat $ROUND6_TMP | grep Carroll | expect_one "<td class=.col-insuper.>T3</td>"

cat $ROUND6_TMP | expect_one Derick
cat $ROUND6_TMP | grep Derick | expect_one "<div class=.inround.>T3</div>"
cat $ROUND6_TMP | grep Derick | expect_one "<td class=.col-ingroup.>T3</td>"
cat $ROUND6_TMP | grep Derick | expect_one "<div class=.insuper.>T3</div>"
cat $ROUND6_TMP | grep Derick | expect_one "<td class=.col-insuper.>T3</td>"

rm $ROUND6_TMP

curl_text "export.php" | sed -n -e '/START_JSON/,/END_JSON/ p' | tail -2 | head -1 | \
    expect_one '["T1","101","Adolfo \"Dolf\" Asher","","Lions \u0026 Tigers","T1","4","12","1st","3rd"]'
curl_text "export.php" | sed -n -e '/START_JSON/,/END_JSON/ p' | tail -2 | head -1 | \
    expect_one '["T1","141","Jesse Jara","","Lions \u0026 Tigers","T1","4","12","1st","3rd"]'
curl_text "export.php" | sed -n -e '/START_JSON/,/END_JSON/ p' | tail -2 | head -1 | \
    expect_one '["T3","111","Carroll Cybulski","Vroom","Lions \u0026 Tigers","T3","4","8","2nd","4th"]'
curl_text "export.php" | sed -n -e '/START_JSON/,/END_JSON/ p' | tail -2 | head -1 | \
    expect_one '["T3","121","Derick Dreier","","Lions \u0026 Tigers","T3","4","8","2nd","4th"]'
