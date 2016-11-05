#! /bin/sh

# This builds the derby-timer.jar and copies to /opt/derbynet.

# TODO Run the jar as a headless service, so just plug in the hardware and you're off:
# TODO java -jar /opt/derbynet/derby-timer.jar -x localhost

. `dirname $0`/common.sh

mkdir /opt/derbynet 2>/dev/null

######## Assumed packages
( dpkg -l | grep '^.i' | grep ant > /dev/null) || \
    apt_get_install ant

# Temporarily change directory while building the jar 
( cd `dirname $0`/../../timer && ant )

cp `dirname $0`/../../timer/dist/lib/derby-timer.jar /opt/derbynet

