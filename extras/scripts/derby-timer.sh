#! /bin/sh

TIMER_ARGS=

test -f /etc/derbynet.conf  && . /etc/derbynet.conf
test -f /boot/derbynet.conf && . /boot/derbynet.conf

# Wait for server and network to come up
CONTACT_OK=0
while [ $CONTACT_OK -eq 0 ]; do
    echo Attempting to contact $DERBYNET_SERVER
    curl --insecure --connect-timeout 6 --location --silent \
         "$DERBYNET_SERVER/index.php" > /dev/null && \
        CONTACT_OK=1
done
echo Starting timer jar

java -jar /usr/bin/derby-timer.jar $TIMER_ARGS $DERBYNET_SERVER
