#! /bin/bash
#
# Set environment variable PASSWORDS_FILE to point to a file containing, e.g.,
#
#RaceCoordinator:doyourbest
#RaceCrew:murphy
#
#

if [ "$BASE_URL" = "" ]; then
	echo Base URL required!
	exit 1
fi

OUTPUT_CURL="`dirname $0`/output.curl"
DEBUG_CURL="`dirname $0`/debug.curl"
COOKIES_CURL="`dirname $0`/cookies.curl"

function header() {
    echo '###################### ' `caller 1 | cut -f3 -d\ ` ' #######################'
}
[ "$NOANNOUNCE" = "" ] && header

function stacktrace() {
    while caller $((n++)); do :; done;
}

function curl_get() {
	echo ' ' ' ' ' ' $1 >&2
	echo    >> $OUTPUT_CURL
	echo $1 >> $OUTPUT_CURL
	echo    >> $OUTPUT_CURL
	curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/$1 | tee $DEBUG_CURL \
        | sed -e 's/&nbsp;/ /g' | xmllint --format - | tee -a $OUTPUT_CURL
    #curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/$1 | sed -e 's/&nbsp;/ /g' | xmllint --format -
}

function curl_get_amper() {
	echo '     ' $1 >&2
	echo    >> $OUTPUT_CURL
	echo $1 >> $OUTPUT_CURL
	echo    >> $OUTPUT_CURL
	curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/$1 | tee $DEBUG_CURL \
		| grep -v '&' | xmllint --format - | tee -a $OUTPUT_CURL
}

# Fetch a page, but don't try to parse it as XML/HTML
function curl_text() {
	echo ' ' ' ' ' ' $1 >&2
	echo    >> $OUTPUT_CURL
	echo $1 >> $OUTPUT_CURL
	echo    >> $OUTPUT_CURL
	curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/$1 | tee $DEBUG_CURL \
        | sed -e 's/&nbsp;/ /g' | tee -a $OUTPUT_CURL
}

function curl_getj() {
	[ "$NOANNOUNCE" = "" ] && echo ' ' ' ' ' ' $1 >&2
	echo    >> $OUTPUT_CURL
	echo $1 >> $OUTPUT_CURL
	echo    >> $OUTPUT_CURL
	curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/$1 | tee $DEBUG_CURL \
        | tee -a $OUTPUT_CURL
}

function curl_post() {
	echo ' ' ' ' ' ' post $1 $2 >&2
	echo    >> $OUTPUT_CURL
	echo post $1 $2 >> $OUTPUT_CURL
	echo    >> $OUTPUT_CURL
	curl --location -d "$2" -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/$1 | tee $DEBUG_CURL \
		| xmllint --format - | tee -a $OUTPUT_CURL
}

function curl_postj() {
	[ "$NOANNOUNCE" = "" ] && echo ' ' ' ' ' ' post $1 $2 >&2
	echo    >> $OUTPUT_CURL
	echo post $1 $2 >> $OUTPUT_CURL
	echo    >> $OUTPUT_CURL

    if [[ -n "$DBCHECK" ]] ; then
        if [[ ! "$2" =~ nodata ]] ; then
            curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL "$BASE_URL/action.php?query=database.check" \
                | tee $DEBUG_CURL | check_jsuccess
        fi
    fi

	curl --location -d "$2" -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/$1 | tee $DEBUG_CURL \
		| tee -a $OUTPUT_CURL
}

# curl_photo $1=url_tail $2=MD5digest
function curl_photo() {
    echo ' ' ' ' ' ' photo $1 expected to yield $2 >&2
    echo    >> $OUTPUT_CURL
    echo photo $1 $2 >> $OUTPUT_CURL
    echo    >> $OUTPUT_CURL
    OK=1
	PHOTO_MD5=`curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/photo.php/$1 | md5`
    echo $PHOTO_MD5 | tee $DEBUG_CURL | tee -a $OUTPUT_CURL | grep -c $2 > /dev/null || OK=0
    if [ $OK -eq 0 ]; then
        test_fails Wrong photo result: $PHOTO_MD5
    fi
}

# curl_photo checks the MD5 digest of the photo that's returned, but for many photo manipulations,
# the digest depends on the exact PHP and library versions used.  curl_photo_any just tests that
# we get some kind of picture and not a failure.
#
# curl_photo_any $1=url_tail
function curl_photo_any() {
    echo ' ' ' ' ' ' photo_any $1 >&2
    echo    >> $OUTPUT_CURL
    echo photo $1 >> $OUTPUT_CURL
    echo    >> $OUTPUT_CURL

    COUNT=`curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/photo.php/$1 | wc -c`
    if [ $COUNT -lt 1000 ]; then
        test_fails No photo result for $1
    fi
}

function curl_put_snapshot() {
    echo ' ' ' ' ' ' snapshot.put $1 >&2
    echo    >> $OUTPUT_CURL
    echo snapshot.put $1 >> $OUTPUT_CURL
    echo    >> $OUTPUT_CURL
    curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/action.php \
        -X POST -F snapshot="@$1" -F action=snapshot.put | tee $DEBUG_CURL | \
    xmllint --format - | tee -a $OUTPUT_CURL
}

function user_login() {
	# $1 = user name
    PWD=$(sed -n -e "s/^$1://p" "${PASSWORDS_FILE:-`dirname $0`/default.passwords}")
	echo ' ' ' ' ' ' login $1 >&2
	echo    >> $OUTPUT_CURL
	echo login $1 >> $OUTPUT_CURL
	echo    >> $OUTPUT_CURL
	curl --location -d "action=role.login&name=$1&password=$PWD" \
        -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/action.php | tee $DEBUG_CURL \
		| tee -a $OUTPUT_CURL | check_jsuccess
}

function user_login_coordinator() {
    user_login RaceCoordinator
}

function user_login_crew() {
    user_login RaceCrew
}

function user_login_timer() {
    user_login Timer
}

function user_login_photo() {
    user_login Photo
}

function user_logout() {
	curl_postj action.php "action=role.login" | check_jsuccess logout
}

function test_fails() {
    tput setaf 1  # red text
	echo TEST FAILURE: $*
    stacktrace
	echo RECEIVED:
	head -n 100 $DEBUG_CURL
    tput setaf 0  # black text
    exit 1
}

function check_success() {
	# Expecting stdin
    OK=1
	grep -c "<success[ />]" > /dev/null || OK=0 
	if [ $OK -eq 0 ]; then
        test_fails
	fi
}

function check_jsuccess() {
	# Expecting stdin
    OK=1
    jq -s -e ' length > 0 and (map(.outcome.summary == "success") | all)' >/dev/null || OK=0
	if [ $OK -eq 0 ]; then
        echo Tail failed
        test_fails
	fi
}

# Some actions are expected to fail, i.e., produce a 
# <failure ...>...</failure>
# response.  The test fails if that's not the case.
function check_failure() {
	# Expecting stdin
    OK=0
	grep -c "<failure[ />]" > /dev/null && OK=1
	if [ $OK -eq 0 ]; then
        test_fails EXPECTING ACTION TO FAIL
	fi
}

function check_jfailure() {
	# Expecting stdin
    OK=0
    jq -e '.outcome.summary == "failure"' >/dev/null && OK=1
	if [ $OK -eq 0 ]; then
        test_fails EXPECTING ACTION TO FAIL
	fi
}

function expect_count {
	# Expecting stdin
	if [ "`grep -c "$1"`" -ne $2 ]; then
        test_fails expecting $2 occurrences of $1
	fi
}

function expect_one {
	if [ "`grep -c "$1"`" -ne 1 ]; then
        test_fails expecting an occurrence of $1
	fi
}

function expect_eq {
    if [ "`cat`" != "$1" ]; then
        test_fails expecting string "$1"
    fi
}


# For generating data:
# select roundid, heat,
# carnumber || ':' || (1 + abs(random()) % 3) || '.' ||
#     (abs(random()) % 10) || (abs(random()) % 10) || (abs(random()) % 10) as L1,
# (select carnumber from RaceChart join RegistrationInfo using (racerid)
#     where lane = 2 and roundid = lane1.roundid and heat = Lane1.heat)
#  || ':' || (1 + abs(random()) % 3) || '.' ||
#     (abs(random()) % 10) || (abs(random()) % 10) || (abs(random()) % 10) as L2,
# (select carnumber from RaceChart join RegistrationInfo using (racerid)
#     where lane = 3 and roundid = lane1.roundid and heat = Lane1.heat)
#  || ':' || (1 + abs(random()) % 3) || '.' ||
#     (abs(random()) % 10) || (abs(random()) % 10) || (abs(random()) % 10) as L3,
# (select carnumber from RaceChart join RegistrationInfo using (racerid)
#     where lane = 4 and roundid = lane1.roundid and heat = Lane1.heat)
#  || ':' || (1 + abs(random()) % 3) || '.' ||
#     (abs(random()) % 10) || (abs(random()) % 10) || (abs(random()) % 10) as L4
# from RaceChart lane1 join RegistrationInfo using (racerid)
# where finishtime is null and lane = 1 and roundid = 2 order by heat;

# Confirm what roundid/heat is current and simulate timer interaction for running one heat
# Usage: run_heat <roundid> <heat> <lane1> <lane2> <lane3> <lane4> ?<skip-check_heat_ready>
#  where <laneN> is either just a time, a carnumber plus a time, e.g.,
#  101:3.210, or a hypen for a bye.
function run_heat() {
    SKIP_CHECK_HEAT_READY=0

    if [[ "$1" == "-place" ]] ; then
        shift
        KEY=place
    else
        KEY=lane
    fi
    
    ROUNDID=$1
    HEAT=$2
    shift 2

    curl_getj "action.php?query=poll.coordinator" | \
        jq -e ".[\"current-heat\"] | .[\"now_racing\"] == true and
               .roundid == $ROUNDID and .heat == $HEAT" >/dev/null || \
                   test_fails Wrong heat or not racing

    CARNOS=`jq -c '.racers | map(.carnumber)' $DEBUG_CURL`

    
    SUGGEST_UPDATE=
    # SUGGEST_UPDATE=1
    if [[ "$1" == *:* ]] ; then
        SUGGEST_UPDATE=
    fi

    if [ $SUGGEST_UPDATE ] ; then
        echo `caller 0 | cut -d\  -f3`:`caller 0 | cut -d\  -f1`
        echo -n run_heat $ROUNDID $HEAT ""
    fi

    FINISHED=""
    LANE=1
    BYES=0
    while [ $# -gt 0 ]; do
        if [ "$1" = "x" ] ; then
            SKIP_CHECK_HEAT_READY=1
        elif [[ "$1" == "-" ]] ; then
            # Lane not occupied.  CARNOS won't have a blank, so adjust BYES
            let BYES+=1
            let LANE+=1
            if [ $SUGGEST_UPDATE ] ; then echo -n " -" ; fi
        elif [[ "$1" == *:* ]] ; then
            EXPECTED_CARNO=$(echo "$1" | cut -d: -f1)
            TIME=$(echo "$1" | cut -d: -f2)

            ACTUAL_CARNO=$(echo $CARNOS | jq -j ".[$LANE-$BYES-1]")

            if [[ "$EXPECTED_CARNO" != "$ACTUAL_CARNO" ]] ; then
                test_fails Wrong car number in lane $LANE: \
                           expected $EXPECTED_CARNO instead of $ACTUAL_CARNO
            fi

            FINISHED="$FINISHED&$KEY$LANE=$TIME"
            let LANE+=1
        else
            FINISHED="$FINISHED&lane$LANE=$1"
            if [ $SUGGEST_UPDATE ] ; then echo -n " $(echo $CARNOS | jq -j ".[$LANE-$BYES-1]"):$1" ; fi
            let LANE+=1
        fi
        shift
    done
    if [ $SUGGEST_UPDATE ] ; then echo ; fi

    curl_post action.php "action=timer-message&message=STARTED" | check_success
    curl_post action.php "action=timer-message&message=FINISHED$FINISHED" | check_success
    if [ $SKIP_CHECK_HEAT_READY -eq 0 ] ; then
        cat $DEBUG_CURL | expect_one "<heat-ready[ />]"
    fi
}

# Usage: staged_heat <lane1-carno> <lane2-carno> <lane3-carno> <lane4-carno>
#  "Bye" lanes are given as 0's
# Usage: staged_heat <lane1-carno> <lane2-carno> <lane3-carno> <lane4-carno> <lane5-carno> <lane6-carno>
function staged_heat6() {
    curl_getj "action.php?query=poll.coordinator" | \
        jq -e ".racers | \
            ($1 == 0 or map(select ( .lane == 1 ))[0].carnumber == $1) and \
            ($2 == 0 or map(select ( .lane == 2 ))[0].carnumber == $2) and \
            ($3 == 0 or map(select ( .lane == 3 ))[0].carnumber == $3) and \
            ($4 == 0 or map(select ( .lane == 4 ))[0].carnumber == $4) and \
            ($5 == 0 or map(select ( .lane == 5 ))[0].carnumber == $5) and \
            ($6 == 0 or map(select ( .lane == 6 ))[0].carnumber == $6)" >/dev/null || \
        test_fails
}

Q='"'

# "$1" is a (unique) prefix for the rank name sought
# E.g. WOLF_RANK=$(rankid_of "White")
rankid_of() {
    NOANNOUNCE=1 curl_getj "action.php?query=class.list" \
        | jq ".classes | map(.subgroups) | add | map(select(.name | startswith($Q$1$Q)))[0].rankid"
}
classid_of() {
    NOANNOUNCE=1 curl_getj "action.php?query=class.list" \
        | jq ".classes | map(select(.name | startswith($Q$1$Q)))[0].classid"
}
partitionid_of() {
    NOANNOUNCE=1 curl_getj "action.php?query=poll&values=partitions" \
        | jq ".partitions | map(select(.name | startswith($Q$1$Q)))[0].partitionid"
}
