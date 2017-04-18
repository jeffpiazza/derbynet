#! /bin/sh

# This script is half of a "shoot then scan" photo capture station, this script
# being the "scan" half.  Whenever the barcode reader scans a barcode, it
# uploads the most recently captured photo to the server, identified by the
# barcode.

# The PHOTO_REPO variable must be one of these two keywords:
# PHOTO_REPO=car
PHOTO_REPO=head
AUTOCROP=1
PHOTO_USER=Photo
PHOTO_PASSWORD=flashbulb

BARCODE_SCANNER_DEV=/dev/input/by-id/usb-13ba_Barcode_Reader-event-kbd

# If set to 1, the barcode scan will also check-in the racer
PHOTO_CHECKIN=0

test -f /etc/derbynet.conf  && . /etc/derbynet.conf
test -f /boot/derbynet.conf && . /boot/derbynet.conf

test -n "$1" && DERBYNET_SERVER="$1"

test -z "$DERBYNET_SERVER" && echo "No server specified" && exit 1

COOKIES=`mktemp`

# Ignore error failures
set +e

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

        echo Uploading `readlink last-photo.jpg`
        curl --fail \
             -F action=photo.upload \
             -F MAX_FILE_SIZE=30000000 \
             -F repo=$PHOTO_REPO \
             -F barcode=$BARCODE \
             -F autocrop=$AUTOCROP \
             -F "photo=@`readlink last-photo.jpg`;type=image/jpeg" \
             -b "$COOKIES" -c "$COOKIES" \
            "$DERBYNET_SERVER/action.php" || echo Upload failure
    elif [ "$BARCODE" = "QUITQUITQUIT" ] ; then
        sudo shutdown -h now
    else
        echo Rejecting scanned barcode $BARCODE
    fi
done
