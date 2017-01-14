#! /bin/sh
# Execute this script to present a full-screen browser using Epiphany.
#
# Epiphany is the default browser for Raspbian, but it doesn't seem to support a
# "full screen" mode that clears the desktop decorations (menu bar, etc.).

xset s off
xset -dpms
xset s noblank

test -f /etc/derbynet.conf  && . /etc/derbynet.conf
test -f /boot/derbynet.conf && . /boot/derbynet.conf

# Ensure the networking stack is functional, because the browser will just
# present an error screen if it gets a "no route to host."

CONTACT_OK=0
while [ $CONTACT_OK -eq 0 ]; do
    curl --location --silent "$DERBYNET_SERVER/index.php" > /dev/null && CONTACT_OK=1
done

epiphany-browser --application-mode --profile /home/pi/.config "$DERBYNET_SERVER/kiosk.php"
