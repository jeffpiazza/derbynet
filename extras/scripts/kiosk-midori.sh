#! /bin/sh
# Execute this script to present a full-screen browser using the Midori browser.
# Note that Midori is not installed by default:
#
# sudo apt-get install midori

xset s off
xset -dpms
xset s noblank
test -x /usr/bin/unclutter && unclutter &

test -f /etc/derbynet.conf  && . /etc/derbynet.conf
test -f /boot/derbynet.conf && . /boot/derbynet.conf

test -n "$1" && DERBYNET_SERVER="$1"

# If the browser crashes, we'd like to resume with the same identity, rather
# than as a new browser/kiosk.  We use the CPU serial number to identify
# ourselves.
if [ -z "$ADDRESS" -a -f /proc/cpuinfo ] ; then
    ADDRESS=$(sed -n -e 's/^Serial[ \t:]*0*\(.*\)/\1/p' /proc/cpuinfo)
fi

# Ensure the networking stack is functional, because the browser will just
# present an error screen if it gets a "no route to host."

CONTACT_OK=0
while [ $CONTACT_OK -eq 0 ]; do
    curl --location --silent "$DERBYNET_SERVER/index.php" > /dev/null && CONTACT_OK=1
done

# While loop allows recovery from browser crashes
while true ; do
    test -x /usr/bin/logger && logger -t kiosk Starting midori browser
    midori --execute Fullscreen --app "$DERBYNET_SERVER/kiosk.php?address=$ADDRESS"
done
