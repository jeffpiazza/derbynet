#! /bin/bash
BASE_URL=$1
set -e -E -o pipefail
source `dirname $0`/common.sh
cat >anonymous.index.tmp <<EOF
        <form method="link" action="ondeck.php">
        <form method="link" action="racer-results.php">
        <form method="link" action="export.php">
        <form method="link" action="about.php">
        <form method="link" action="login.php">
EOF

cat >racecrew.index.tmp <<EOF
          <form method="link" action="checkin.php">
          <form method="get" action="photo-thumbs.php">
          <form method="get" action="photo-thumbs.php">
          <form method="link" action="ondeck.php">
          <form method="link" action="racer-results.php">
          <form method="link" action="standings.php">
          <form method="link" action="export.php">
          <form method="link" action="about.php">
          <form method="link" action="login.php">
EOF

cat >coordinator.index.tmp <<EOF
        <form method="link" action="coordinator.php">
        <form method="link" action="awards-presentation.php">
        <form method="link" action="checkin.php">
        <form method="get" action="photo-thumbs.php">
        <form method="get" action="photo-thumbs.php">
        <form method="link" action="ondeck.php">
        <form method="link" action="racer-results.php">
        <form method="link" action="standings.php">
        <form method="link" action="export.php">
        <form method="link" action="settings.php">
        <form method="link" action="setup.php">
        <form method="link" action="import-roster.php">
        <form method="link" action="class-editor.php">
        <form method="link" action="about.php">
        <form method="link" action="login.php">
EOF

## Every page, and every action

user_logout

OK=1
( curl_get index.php | grep '<form' | diff -b - anonymous.index.tmp ) || OK=0
if [ $OK -eq 0 ]; then
    curl_get index.php | grep '<form'
    test_fails Anonymous index page
fi

curl_post action.php "action=kiosk.assign" | check_failure

curl_post action.php "action=result.delete" | check_failure
curl_post action.php "action=racer.edit" | check_failure
curl_post action.php "action=result.write" | check_failure
curl_post action.php "action=racer.import" | check_failure
curl_post action.php "action=initnumbers" | check_failure

# curl_post action.php "action=login" | check_failure
curl_post action.php "action=racer.new" | check_failure
curl_post action.php "action=racer.pass" | check_failure
curl_post action.php "action=photo" | check_failure
# TODO Replay application registers itself without credentials, a security weakness.
# curl_post action.php "action=register-replay" | check_failure
curl_post action.php "action=replay-test" | check_failure
curl_post action.php "action=schedule.reschedule" | check_failure
curl_post action.php "action=schedule.generate" | check_failure
curl_post action.php "action=select-heat" | check_failure
curl_post action.php "action=timer-message" | check_failure
curl_post action.php "action=award.xbs" | check_failure

# Queries don't answer "<success/>" or "<failure/>", so there's really
# nothing to check other than that they parse as XML.
curl_get "action.php?query=class.list" > /dev/null
curl_get "action.php?query=poll.coordinator" > /dev/null
curl_get "action.php?query=kiosk.poll" > /dev/null
curl_get "action.php?query=poll.kiosk.all" > /dev/null
curl_get "action.php?query=roles"  > /dev/null
curl_get "action.php?query=poll.ondeck" > /dev/null
curl_get "action.php?query=poll.now-racing" > /dev/null

user_login_coordinator
( curl_get index.php | grep '<form' | diff -b - coordinator.index.tmp ) || OK=0
if [ $OK -eq 0 ]; then
    curl_get index.php | grep '<form'
    test_fails Coordinator index page
fi
user_logout

( curl_get index.php | grep '<form' | diff -b - anonymous.index.tmp ) || OK=0
if [ $OK -eq 0 ]; then
    test_fails Anonymous index page again
fi

user_login_crew
( curl_get index.php | grep '<form' | diff -b - racecrew.index.tmp ) || OK=0
if [ $OK -eq 0 ]; then
    curl_get index.php | grep '<form'
    test_fails Race crew index page
fi
user_logout

( curl_get index.php | grep '<form' | diff -b - anonymous.index.tmp ) || OK=0
if [ $OK -eq 0 ]; then
    curl_get index.php | grep '<form'
    test_fails Anonymous third time
fi

rm *.index.tmp

