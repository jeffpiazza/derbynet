#! /bin/sh
# This script uses uses a bar code scanner and a tethered camera to automate photo capture for
# your pinewood derby event.
#
# Usage: photostand.sh <web server base URL>
# E.g.,  photostand.sh http://192.168.1.37/derbynet
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

SERVER="$1"
# The REPO variable must be one of these two keywords:
REPO=car
# REPO=head

test -z "$SERVER" && echo "No server specified" && exit 1

BARCODE_SCANNER_DEV=/dev/input/by-id/usb-Megawin_Technology_Inc._USB_Keyboard-event-kbd

COOKIES=`mktemp`

# Ignore error failures
set +e

curl --location -d "action=login&name=Photo&password=flashbulb" \
        -s -b "$COOKIES" -c "$COOKIES" $SERVER/action.php

while true ; do
    CAR_NO=`barcode $BARCODE_SCANNER_DEV | grep -e "^PWD[0-9]*$" | sed -e "s/PWD//"`
    if [ "$CAR_NO" ] ; then

	# Assumes there's only one camera attached
	chdkptp -c -e"rec" -e"remoteshoot Car$CAR_NO"
    # Alternatively:
    # gphoto2 --filename "Car$CAR_NO" --capture-image-and-download

	curl -s -F action=photo.upload \
             -F MAX_FILE_SIZE=30000000 \
             -F repo=car \
             -F carnumber=$CAR_NO \
             -F "photo=@Car$CAR_NO.jpg;type=image/jpeg" \
             -b "$COOKIES" -c "$COOKIES" \
            "$SERVER/action.php"
fi
done
