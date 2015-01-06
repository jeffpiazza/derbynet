#! /bin/sh

if [ "$BASE_URL" = "" ]; then
	echo Base URL required!
	exit
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
}

function curl_get_amper() {
	echo '     ' $1 >&2
	echo    >> $OUTPUT_CURL
	echo $1 >> $OUTPUT_CURL
	echo    >> $OUTPUT_CURL
	curl --location -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/$1 | tee $DEBUG_CURL \
		| grep -v '&' | xmllint --format - | tee -a $OUTPUT_CURL
}

function curl_post() {
	echo ' ' ' ' ' ' post $1 $2 >&2
	echo    >> $OUTPUT_CURL
	echo post $1 $2 >> $OUTPUT_CURL
	echo    >> $OUTPUT_CURL
	curl --location -d "$2" -s -b $COOKIES_CURL -c $COOKIES_CURL $BASE_URL/$1 | tee $DEBUG_CURL \
		| xmllint --format - | tee -a $OUTPUT_CURL
}

function user_login() {
	# $1 = user name
	# $2 = password
	curl_post action.php "action=login&name=$1&password=$2" | check_success "login"
}

function user_logout() {
	curl_post action.php "action=login" | check_success logout
}

function test_fails() {
    tput setaf 1
	echo TEST FAILURE: $*
    stacktrace
	echo BEGIN RESPONSE
	cat $DEBUG_CURL
	echo END RESPONSE
    tput setaf 0
}

function check_success() {
	# Expecting stdin
	grep -c "<success[ />]" > /dev/null
	if [ $? -ne 0 ]; then
        test_fails
	fi
}

# Some actions are expected to fail, i.e., produce a 
# <failure ...>...</failure>
# response.  The test fails if that's not the case.
function check_failure() {
	# Expecting stdin
	grep -c "<failure[ />]" > /dev/null
	if [ $? -ne 0 ]; then
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
	if [ "`grep -c $1`" -ne $2 ]; then
        test_fails expecting $2 occurrences of $1
	fi
}

function expect_one {
	if [ "`grep -c $1`" -ne 1 ]; then
        test_fails expecting an occurrence of $1
	fi
}
    