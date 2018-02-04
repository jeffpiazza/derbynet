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

function curl_post() {
	echo ' ' ' ' ' ' post $1 $2 >&2
	echo    >> $OUTPUT_CURL
	echo post $1 $2 >> $OUTPUT_CURL
	echo    >> $OUTPUT_CURL
	curl --location -d "$2" -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/$1 | tee $DEBUG_CURL \
		| xmllint --format - | tee -a $OUTPUT_CURL
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

function check_heat_ready() {
	# Expecting stdin
	grep -c "<heat-ready[ />]" $DEBUG_CURL > /dev/null
	if [ $? -ne 0 ]; then
        test_fails EXPECTING HEAT-READY
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
    
