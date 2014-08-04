#! /bin/sh

cat >anonymous.index.tmp <<EOF
        <form method="link" action="ondeck.php">
        <form method="link" action="racer-results.php">
        <form method="link" action="login.php">
EOF

cat >coordinator.index.tmp <<EOF
        <form method="link" action="checkin.php">
        <form method="link" action="photo-thumbs.php">
        <form method="link" action="ondeck.php">
        <form method="link" action="racer-results.php">
        <form method="link" action="coordinator.php">
        <form method="link" action="awards.php">
        <form method="link" action="settings.php">
        <form method="link" action="import-roster.php">
        <form method="link" action="login.php">
EOF

cat >racecrew.index.tmp <<EOF
        <form method="link" action="checkin.php">
        <form method="link" action="ondeck.php">
        <form method="link" action="racer-results.php">
        <form method="link" action="awards.php">
        <form method="link" action="login.php">
EOF

function curl_get() {
	echo ' ' ' ' ' ' $1 >&2
	echo    >> output.curl
	echo $1 >> output.curl
	echo    >> output.curl
	curl --location -s -b cookies.curl -c cookies.curl $BASE_URL/$1 | tee debug.curl \
		| sed -e 's/&nbsp;/ /g' | xmllint --format - | tee -a output.curl
}

function curl_get_amper() {
	echo '     ' $1 >&2
	echo    >> output.curl
	echo $1 >> output.curl
	echo    >> output.curl
	curl --location -s -b cookies.curl -c cookies.curl $BASE_URL/$1 | tee debug.curl \
		| grep -v '&' | xmllint --format - | tee -a output.curl
}

function curl_post() {
	echo ' ' ' ' ' ' post $1 $2 >&2
	echo    >> output.curl
	echo post $1 $2 >> output.curl
	echo    >> output.curl
	curl --location -d $2 -s -b cookies.curl -c cookies.curl $BASE_URL/$1 | tee debug.curl \
		| xmllint --format - | tee -a output.curl
}

function user_login() {
	# $1 = user name
	# $2 = password
	curl_post action.php "action=login&name=$1&password=$2" | check_success "login"
}

function user_logout() {
	curl_post action.php "action=login" | check_success logout
}

function check_success() {
	# Expecting stdin
	grep -c "<success[ />]" > /dev/null
	if [ $? -ne 0 ]; then
        tput setaf 1
		echo FAILURE
		echo BEGIN RESPONSE
		cat debug.curl
		echo END RESPONSE
        tput setaf 0
	fi
}

# Some actions are expected to fail, i.e., produce a 
# <failure ...>...</failure>
# response.  The test fails if that's not the case.
function check_failure() {
	# Expecting stdin
	grep -c "<failure[ />]" > /dev/null
	if [ $? -ne 0 ]; then
        tput setaf 1
		echo TEST FAILURE: EXPECTING ACTION TO FAIL
		echo BEGIN RESPONSE
		cat debug.curl
		echo END RESPONSE
        tput setaf 0
	fi
}

function expect_count {
	# Expecting stdin
	if [ "`grep -c $1`" -ne $2 ]; then
        tput setaf 1
		echo FAILURE expecting $2 occurrences of $1
		echo BEGIN RESPONSE
		cat debug.curl
		echo END RESPONSE
        tput setaf 0
	fi
}

function expect_one {
	if [ "`grep -c $1`" -ne 1 ]; then
        tput setaf 1
		echo FAILURE expecting an occurrence of $1
		echo BEGIN RESPONSE
		cat debug.curl
		echo END RESPONSE
        tput setaf 0
	fi
}
    
