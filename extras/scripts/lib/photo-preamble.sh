#! /bin/sh

# This "preamble" script defines default values for a bunch of environment variables,
# and then sources:
#
#    /etc/derbynet.conf
#    /boot/derbynet.conf
#
# to override with customized variables.

# announce() is a scripting hook to provide feedback to the user in response to
# various events encountered.  Takes one argument, the "event" name.
#
# Local installations should override by providing an alternate definition in
# /etc/derbynet.conf or similar:
#
#    #! /bin/sh
#    unset announce
#    announce() { ... }
#
announce() {
    echo Announce: $1
    case $1 in
        initializing) ;;
        terminating) ;;
        idle) ;;
        barcode-read) ;;
        sending) ;;
        no-network) ;;
        login-ok) ;;
        no-scanner) ;;
        no-camera) ;;
        capture-ok) ;;
        checkin-failed) ;;
        success) ;;
        upload-ok-but-checkin-failed) ;;
        upload-failed) ;;
        speed-good) ;;
        speed-fair) ;;
        speed-poor) ;;
        unrecognized-barcode) ;;
    esac
}

# The PHOTO_REPO variable must be one of these two keywords:
PHOTO_REPO=car
# PHOTO_REPO=head

# The local directory in which to store today's captured photos.
# If this variable is not overridden (still empty) after sourcing config files,
# will be set based on date.
PHOTO_DIR=

# Auto-cropping automatically crops head shots; probably not what you want for
# car photos.
AUTOCROP=0
# AUTOCROP=1

PHOTO_USER=Photo
PHOTO_PASSWORD=flashbulb

# If set to 1, the barcode scan will also check-in the racer
PHOTO_CHECKIN=0

# Raspberry Pi and similar devices don't keep time while they're unplugged.  For
# these cases, the derbynet server reports the current time in a login response.
# If this variable is set, the do_login function will attempt to adjust the
# system clock to match the server-reported time.
ADJUST_CLOCK=0
# ADJUST_CLOCK=1

# Command invoked just before running chdkptp to capture a photo.
# Override to adjust zoom, etc., if desired
# E.g. chdkptp -c -e"rec" -e"luar set_zoom(50)"
prepare_camera_before_shot() {
    :
}

test -f /etc/derbynet.conf  && . /etc/derbynet.conf
test -f /boot/derbynet.conf && . /boot/derbynet.conf

announce initializing

# $1 is from the calling script
test -n "$1" && DERBYNET_SERVER="$1"

test -z "$DERBYNET_SERVER" && echo "No server specified" && exit 1

COOKIES=`mktemp`

# Ignore error failures
set +e

announce idle
