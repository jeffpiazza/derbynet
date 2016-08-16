#! /bin/sh
#
# This script initializes a Raspberry Pi 3 running Raspbian to run a
# private wireless network.
#
# The RPi3's built-in wifi is capable of simultaneously acting as an
# access point (for the private network) and a regular station on
# another wireless network.
#
# We configure the device so that, if the device is within range of a
# known wireless network (according to
# /etc/wpa_supplicant/wpa_supplicant.conf), or if the device's
# eithernet port is connected, the pi will route traffic from the
# private network to the other connection.

. `dirname $0`/common.sh

SERIAL=$(cat /proc/cpuinfo | sed -ne 's/Serial[ \t]*:[ \t]*0*//p')
UNIQ_NAME=derbynet-pi-$SERIAL

mkdir /opt/derbynet 2>/dev/null

######## Assumed packages
( dpkg -l | grep '^.i' | grep hostapd > /dev/null) || \
    apt_get_install hostapd

( dpkg -l | grep '^.i' | grep dnsmasq > /dev/null) || \
    apt_get_install dnsmasq

( dpkg -l | grep '^.i' | grep nginx > /dev/null) || \
    apt_get_install nginx

######## /etc/network/interfaces

if [ ! -f /etc/network/interfaces.d/wlan1-access-point ] ; then
    cat >/etc/network/interfaces.d/wlan1-access-point <<EOF
# Configure virtual interface wlan1 with fixed ip.
# See /opt/derbynet
allow-hotplug wlan1
iface wlan1 inet static
  address 10.10.10.1
  netmask 255.255.255.0
EOF
fi

grep /etc/network/derbynet /etc/network/interfaces > /dev/null || \
    sed -i.pre-derbynet \
	-e 's/^\(allow-hotplug wlan1\)/#\1/' \
        -e '/^iface wlan1/,/^\s*$/ s/^\(\s*[^ ]\)/#\1/' \
        -e '$ a#\
# See /etc/network/interfaces.d' \
        /etc/network/interfaces

######## Virtual access point interface and routing rules

cat <<EOF >/etc/sysctl.d/98-forwarding.conf
net.ipv4.ip_forward=1
EOF

sysctl net.ipv4.ip_forward=1 > /dev/null

cat <<EOF >/opt/derbynet/add-access-point
#! /bin/sh

# Creates a virtual interface to act as an access point with a
# MAC address that differs from the hardware interface's MAC.

iw dev wlan0 interface add wlan1 type __ap
ip link set wlan1 address 10:10:10:ff:ff:ff
EOF
chmod +x /opt/derbynet/add-access-point

cat <<EOF >/opt/derbynet/iptables-rules
#! /bin/sh

# This script provided by derbynet.

# wlan0, eth0 are (potentially) connected to the broader internet,
# probably through another router.
#
# wlan1 is our private wireless LAN, 10.10.10.1.

# Flush all the rules in filter and nat tables
iptables --table filter --flush
iptables --table nat    --flush

# Delete all chains that are not in default filter and nat table
iptables --table filter --delete-chain
iptables --table nat    --delete-chain

# Set up IP FORWARDing and Masquerading (via either eth0 or wlan0)
iptables --table nat    --append POSTROUTING --out-interface wlan0 -j MASQUERADE
iptables --table nat    --append POSTROUTING --out-interface eth0  -j MASQUERADE
iptables --table filter --append FORWARD      --in-interface wlan1 -j ACCEPT
EOF
chmod +x /opt/derbynet/iptables-rules

grep iptables-rules /etc/rc.local > /dev/null || \
    sed -i.pre-derbynet \
	-e '/^exit 0/ i /opt/derbynet/add-access-point\
/opt/derbynet/iptables-rules' \
	/etc/rc.local

/opt/derbynet/add-access-point
/opt/derbynet/iptables-rules

######## /etc/hostapd/hostapd.conf

sed -i -e 's!#DAEMON_CONF=""!DAEMON_CONF="/etc/hostapd/hostapd.conf"!' /etc/default/hostapd

if [ -f /etc/hostapd/hostapd.conf -a ! -f /etc/hostapd/hostapd.conf.pre-derbynet ] ; then
    cp /etc/hostapd/hostapd.conf /etc/hostapd/hostapd.conf.pre-derbynet
fi

gunzip -dc /usr/share/doc/hostapd/examples/hostapd.conf.gz | \
    sed	-e 's/^interface=wlan0/interface=wlan1/' \
	-e "s/^ssid=.*/ssid=$UNIQ_NAME/" \
	-e 's!^channel=.*!#This line gets modified by /opt/derbynet/channel-match script\
channel=1!' \
	> /etc/hostapd/hostapd.conf

service hostapd restart

######## channel-match cron job

cat >/opt/derbynet/channel-match <<EOF
#! /bin/sh
# The RPi3's built-in wireless can support more than one virtual interface,
# but they have to operate on the same channel.  This script updates the
# hostapd configuration file to use the same channel that the wireless
# station interface is using.

CHANNEL=`iwgetid wlan0 --channel | sed -e 's/^.*Channel:\([0-9]*\)/\1/'`
PREV_CHANNEL=`sed -n -e 's/^channel=\([0-9]*\)/\1/p' /etc/hostapd/hostapd.conf`

test -n "$CHANNEL" || exit 0
test "$CHANNEL" != "$PREV_CHANNEL" || exit 0

logger -t channel-match Previously configured channel was $PREV_CHANNEL
logger -t channel-match wlan0 channel is now $CHANNEL

sed -i /etc/hostapd/hostapd.conf -e "s/^channel=[0-9]*/channel=$CHANNEL/"

logger -t channel-match Restarting hostapd
service hostapd restart

EOF


cat >/etc/cron.d/channel-match <<EOF
 * * * * * root /opt/derbynet/channel-match
EOF

######## dnsmasq

cat <<EOF > /etc/dnsmasq.d/derbynet-portal.conf

# DHCP
dhcp-authoritative
interface=wlan1
dhcp-range=10.10.10.10,10.10.10.200,255.255.255.0,12h
dhcp-option=3,10.10.10.1
log-dhcp

# DNS
address=/derby.net/10.10.10.1
address=/derbynet.jeffpiazza.org/10.10.10.1
EOF

grep derbynet /etc/dnsmasq.conf > /dev/null || \
    sed -i.pre-derbynet \
        -e '$ aconf-file=/etc/dnsmasq.d/derbynet-portal.conf' \
	/etc/dnsmasq.conf

service dnsmasq restart

######## Hostname and Message of the Day

echo $UNIQ_NAME > /etc/hostname
hostname "$UNIQ_NAME"

cat <<EOF | sed -e "s/UNIQ_NAME/$UNIQ_NAME/" >/etc/motd
Welcome to UNIQ_NAME
DerbyNet is free software, available at http://jeffpiazza.github.io/derbynet.
EOF

######## Disable vanilla default site
rm -f /etc/nginx/sites-enabled/default 2>/dev/null

######## redirects for all other sites

write_derbynet_snippet
cat <<EOF | sed -e "s/SERVERNAME/$SERVERNAME/" >/etc/nginx/sites-available/redirect  
server {
  listen 10.10.10.1:80 default_server;
  # server_name _;
  return 301 http://derby.net/;
}

server {
  listen 10.10.10.1:80;
  listen 10.10.10.1:443 ssl;
  # TODO
  # include snippets/snakeoil.conf;
  ssl_certificate     /etc/nginx/derbynet/derbynet.pem;
  ssl_certificate_key /etc/nginx/derbynet/derbynet.key;

  server_name SERVERNAME;

  include /etc/nginx/derbynet/server-snippet.conf;
}
EOF
ln -sf /etc/nginx/sites-available/redirect /etc/nginx/sites-enabled

nginx -s reload
