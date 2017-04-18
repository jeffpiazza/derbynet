#! /bin/sh
#
# This script uses uses a bar code scanner and a tethered camera to automate
# photo capture for your pinewood derby event.  See
# http://www.derbytalk.com/viewtopic.php?f=25&t=8253&p=79318 for more
# disccussion.
#
# Usage: photostand.sh <web server base URL>
# E.g.,  photostand.sh http://192.168.1.37/derbynet
#
# If the base URL is not given on the command line, and it's available from
# either /boot/derbynet.conf or /etc/derbynet.conf, that value will be used.
#
# The script logs in to the server when started, then loops waiting for barcodes
# to be scanned.  Check the server's php.ini, particularly
# session.gc_maxlifetime, to be sure that session cookies won't get reclaimed
# before we're done.
#
# Obtain the 'barcode' command:
#
# git clone https://github.com/jeffpiazza/read-barcode.git
# cd read-barcode
# make
# sudo cp barcode /usr/local/bin
#

# The PHOTO_REPO variable must be one of these two keywords:
PHOTO_REPO=car
# PHOTO_REPO=head

# Auto-cropping automatically crops head shots; probably not what you want for
# car photos.
AUTOCROP=0
# AUTOCROP=1

PHOTO_USER=Photo
PHOTO_PASSWORD=flashbulb
BARCODE_SCANNER_DEV=/dev/input/by-id/usb-Megawin_Technology_Inc._USB_Keyboard-event-kbd

# If set to 1, the barcode scan will also check-in the racer
PHOTO_CHECKIN=0

# By default, we use gphoto2, which can talk to most dSLR cameras over USB.
# gphoto2 is available in most linux distributions, or from http://gphoto.org.
#
# An alternative is to use chdkptp (https://app.assembla.com/wiki/show/chdkptp),
# which works with Canon cameras running the Canon Hacker's Development Kit
# (http://chdk.wikia.com/wiki/CHDK).
USE_CHDKPTP=0

test -f /etc/derbynet.conf  && . /etc/derbynet.conf
test -f /boot/derbynet.conf && . /boot/derbynet.conf

test -n "$1" && DERBYNET_SERVER="$1"

test -z "$DERBYNET_SERVER" && echo "No server specified" && exit 1

COOKIES=`mktemp`

# Ignore error failures
set +e

# This gvfs daemon conflicts with chdkptp and gphoto2, and prevents correct
# operation of the script, so kill it if it's running.  Run a loop in the
# background, in case it's still coming up when this script runs.
while true ; do sudo killall gvfs-gphoto2-volume-monitor > /dev/null 2>&1 ; sleep 4s ; done &

# If there are connectivity problems, keep trying until login is successful.
LOGIN_OK=0
while [ $LOGIN_OK -eq 0 ]; do
    echo Logging in to $DERBYNET_SERVER
    curl --location --data "action=login&name=$PHOTO_USER&password=$PHOTO_PASSWORD" \
         --silent --show-error -b "$COOKIES" -c "$COOKIES" -o - \
         "$DERBYNET_SERVER/action.php" \
    | grep -q success \
    && LOGIN_OK=1
    test $LOGIN_OK -eq 0 && sleep 1s
done

echo Successfully logged in

# Connect to camera, set to picture-taking mode.  (This lets operator adjust
# photo composition.)
#
# Assumes there's only one camera attached
if [ $USE_CHDKPTP -ne 0 ] ; then
    echo Activating camera
    chdkptp -c -e"rec"
fi

while true ; do
    BARCODE=`barcode $BARCODE_SCANNER_DEV`
    echo Scanned $BARCODE
    CAR_NO=`echo $BARCODE | grep -e "^PWD" | sed -e "s/^PWD//"`
    if [ "$CAR_NO" ] ; then
        if [ $PHOTO_CHECKIN -ne 0 ] ; then
        echo Checking in racer $BARCODE
            # Check in the racer
            curl --silent -F action=racer.pass \
                 -F barcode=$BARCODE \
                 -F value=1 \
                 -b "$COOKIES" -c "$COOKIES" \
                 "$DERBYNET_SERVER/action.php"
        fi

        echo Capturing photo Car$CAR_NO.jpg
        if [ $USE_CHDKPTP -eq 0 ] ; then
            gphoto2 --filename "Car$CAR_NO" --capture-image-and-download
        else
            chdkptp -c -e"rec" -e"remoteshoot Car$CAR_NO"
        fi

        echo Uploading $BARCODE
        curl --silent -F action=photo.upload \
             -F MAX_FILE_SIZE=30000000 \
             -F repo=$PHOTO_REPO \
             -F barcode=$BARCODE \
             -F autocrop=$AUTOCROP \
             -F "photo=@Car$CAR_NO.jpg;type=image/jpeg" \
             -b "$COOKIES" -c "$COOKIES" \
             "$DERBYNET_SERVER/action.php"
    elif [ "$BARCODE" = "QUITQUITQUIT" ] ; then
        sudo shutdown -h now
    else
        echo Rejecting scanned barcode $BARCODE
    fi
done
