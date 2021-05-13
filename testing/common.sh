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
	curl --location -d "action=login&name=$1&password=$PWD" \
        -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/action.php | tee $DEBUG_CURL \
		| xmllint --format - | tee -a $OUTPUT_CURL | check_success
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
	curl_post action.php "action=login" | check_success logout
}

function test_fails() {
    tput setaf 1  # red text
	echo TEST FAILURE: $*
    stacktrace
	echo BEGIN RESPONSE
	cat $DEBUG_CURL
	echo END RESPONSE
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
    jq -e '.outcome.summary == "success"' >/dev/null || OK=0
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

# Confirm what roundid/heat is current and simulate timer interaction for running one heat
# Usage: run_heat <roundid> <heat> <lane1> <lane2> <lane3> <lane4> ?<skip-check_heat_ready>
function run_heat() {
    ROUNDID=$1
    HEAT=$2
    LANE1=$3
    LANE2=$4
    LANE3=$5
    LANE4=$6
    SKIP_CHECK_HEAT_READY=$7

    curl_getj "action.php?query=json.poll.coordinator" | \
        jq ".[\"current-heat\"] | .[\"now_racing\"] == true and .roundid == $ROUNDID and .heat == $HEAT" | \
        expect_eq true
    
    curl_post action.php "action=timer-message&message=STARTED" | check_success
    curl_post action.php "action=timer-message&message=FINISHED&lane1=$LANE1&lane2=$LANE2&lane3=$LANE3&lane4=$LANE4" | check_success
    if [ -z "$SKIP_CHECK_HEAT_READY" ] ; then
        cat $DEBUG_CURL | expect_one "<heat-ready[ />]"
    fi
}

# Usage: run_heat_place <roundid> <heat> <place1> <place2> <place3> <place4> ?<skip-check_heat_ready>
function run_heat_place() {
    ROUNDID=$1
    HEAT=$2
    PLACE1=$3
    PLACE2=$4
    PLACE3=$5
    PLACE4=$6
    SKIP_CHECK_HEAT_READY=$7

    curl_getj "action.php?query=json.poll.coordinator" | \
        jq ".[\"current-heat\"] | .[\"now_racing\"] == true and .roundid == $ROUNDID and .heat == $HEAT" | \
        expect_eq true

    curl_post action.php "action=timer-message&message=STARTED" | check_success
    curl_post action.php "action=timer-message&message=FINISHED&place1=$PLACE1&place2=$PLACE2&place3=$PLACE3&place4=$PLACE4" | check_success
    if [ -z "$SKIP_CHECK_HEAT_READY" ] ; then
        cat $DEBUG_CURL | expect_one "<heat-ready[ />]"
    fi
}

# Usage: staged_heat <lane1-carno> <lane2-carno> <lane3-carno> <lane4-carno>
function staged_heat4() {
    curl_getj "action.php?query=json.poll.coordinator" | \
        jq ".racers | \
            (\"$1\" == \"-\" or map(select ( .lane == 1 ))[0].carnumber == \"$1\") and \
            (\"$2\" == \"-\" or map(select ( .lane == 2 ))[0].carnumber == \"$2\") and \
            (\"$3\" == \"-\" or map(select ( .lane == 3 ))[0].carnumber == \"$3\") and \
            (\"$4\" == \"-\" or map(select ( .lane == 4 ))[0].carnumber == \"$4\")" | \
        expect_eq true
}

# Usage: staged_heat <lane1-carno> <lane2-carno> <lane3-carno> <lane4-carno> <lane5-carno> <lane6-carno>
function staged_heat6() {
    curl_getj "action.php?query=json.poll.coordinator" | \
        jq ".racers | \
            (\"$1\" == \"-\" or map(select ( .lane == 1 ))[0].carnumber == \"$1\") and \
            (\"$2\" == \"-\" or map(select ( .lane == 2 ))[0].carnumber == \"$2\") and \
            (\"$3\" == \"-\" or map(select ( .lane == 3 ))[0].carnumber == \"$3\") and \
            (\"$4\" == \"-\" or map(select ( .lane == 4 ))[0].carnumber == \"$4\") and \
            (\"$5\" == \"-\" or map(select ( .lane == 5 ))[0].carnumber == \"$5\") and \
            (\"$6\" == \"-\" or map(select ( .lane == 6 ))[0].carnumber == \"$6\")" | \
        expect_eq true
}
