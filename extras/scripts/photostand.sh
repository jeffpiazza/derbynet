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
# To control the camera, this version uses chdkptp
# (https://app.assembla.com/wiki/show/chdkptp), which works with Canon cameras
# running the Canon Hacker's Development Kit (http://chdk.wikia.com/wiki/CHDK).
#
# Alternatively, the gphoto2 command (available in most linux distributions, or
# from http://gphoto.org) can interface to many/most dSLR cameras over USB.
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

test -f /etc/derbynet.conf  && . /etc/derbynet.conf
test -f /boot/derbynet.conf && . /boot/derbynet.conf

test -n "$1" && DERBYNET_SERVER="$1"

test -z "$DERBYNET_SERVER" && echo "No server specified" && exit 1

COOKIES=`mktemp`

# Ignore error failures
set +e

# This gvfs daemon conflicts with chdkptp and prevents correct operation of the script,
# so kill it if it's running
sudo killall gvfs-gphoto2-volume-monitor > /dev/null 2>&1

# If there are connectivity problems, keep trying until login is successful.
LOGIN_OK=0
while [ $LOGIN_OK -eq 0 ]; do
    echo Logging in
    curl --location --data "action=login&name=$PHOTO_USER&password=$PHOTO_PASSWORD" \
         --silent -b "$COOKIES" -c "$COOKIES" -o - \
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
chdkptp -c -e"rec"

while true ; do
    BARCODE=`barcode $BARCODE_SCANNER_DEV | grep -e "^PWD[0-9]*$"`
    CAR_NO=`echo $BARCODE | sed -e "s/PWD//"`
    if [ "$CAR_NO" ] ; then

        if [ $PHOTO_CHECKIN -ne 0 ] ; then
            # Check in the racer
            curl --silent -F action=racer.pass \
                 -F barcode=$BARCODE \
                 -F value=1 \
                 -b "$COOKIES" -c "$COOKIES" \
                 "$DERBYNET_SERVER/action.php"
        fi

        chdkptp -c -e"rec" -e"remoteshoot Car$CAR_NO"
        # Alternatively:
        # gphoto2 --filename "Car$CAR_NO" --capture-image-and-download

        curl --silent -F action=photo.upload \
             -F MAX_FILE_SIZE=30000000 \
             -F repo=$PHOTO_REPO \
             -F carnumber=$CAR_NO \
             -F autocrop=$AUTOCROP \
             -F "photo=@Car$CAR_NO.jpg;type=image/jpeg" \
             -b "$COOKIES" -c "$COOKIES" \
            "$DERBYNET_SERVER/action.php"
    fi
done
