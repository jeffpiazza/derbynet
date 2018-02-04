#! /bin/sh
# Execute this script to present a full-screen browser using Epiphany.
#
# Epiphany is the default browser for Raspbian, but it doesn't seem to support a
# "full screen" mode that clears the desktop decorations (menu bar, etc.).

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
    test -x /usr/bin/logger && logger -t kiosk Starting epiphany browser
    epiphany-browser --application-mode --profile /home/pi/.config "$DERBYNET_SERVER/kiosk.php?address=$ADDRESS"
done
