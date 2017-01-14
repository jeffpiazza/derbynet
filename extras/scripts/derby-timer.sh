#! /bin/sh

test -f /etc/derbynet.conf  && . /etc/derbynet.conf
test -f /boot/derbynet.conf && . /boot/derbynet.conf

java -jar /usr/bin/derby-timer.jar -u $TIMER_USER -p $TIMER_PASSWORD $DERBYNET_SERVER
