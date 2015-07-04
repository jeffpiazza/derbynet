#! /bin/bash
BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh

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

## Every page, and every action

user_logout

curl_get index.php | grep '<form' | diff - anonymous.index.tmp

curl_post action.php "action=assign-kiosk" | check_failure

curl_post action.php "action=delete-results" | check_failure
curl_post action.php "action=edit-racer" | check_failure
curl_post action.php "action=heat-results" | check_failure
curl_post action.php "action=import" | check_failure
curl_post action.php "action=initnumbers" | check_failure

# curl_post action.php "action=login" | check_failure
curl_post action.php "action=new-racer" | check_failure
curl_post action.php "action=pass" | check_failure
curl_post action.php "action=photo" | check_failure
# TODO Replay application registers itself without credentials, a security weakness.
# curl_post action.php "action=register-replay" | check_failure
curl_post action.php "action=replay-test" | check_failure
curl_post action.php "action=reschedule" | check_failure
curl_post action.php "action=schedule" | check_failure
curl_post action.php "action=select-heat" | check_failure
curl_post action.php "action=timer-message" | check_failure
curl_post action.php "action=xbs" | check_failure

# Queries don't answer "<success/>" or "<failure/>", so there's really
# nothing to check other than that they parse as XML.
curl_get "action.php?query=classes" > /dev/null
curl_get "action.php?query=coordinator-poll" > /dev/null
curl_get "action.php?query=kiosk-poll" > /dev/null
curl_get "action.php?query=update-summary" > /dev/null
curl_get "action.php?query=watching" > /dev/null

user_login_coordinator
curl_get index.php | grep '<form' | diff - coordinator.index.tmp
user_logout

curl_get index.php | grep '<form' | diff - anonymous.index.tmp

user_login_crew
curl_get index.php | grep '<form' | diff - racecrew.index.tmp
user_logout

curl_get index.php | grep '<form' | diff - anonymous.index.tmp

rm *.index.tmp

