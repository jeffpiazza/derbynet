#! /bin/sh

. `dirname $0`/common.sh

SERIAL=$(cat /proc/cpuinfo | sed -ne 's/Serial[ \t]*:[ \t]*0*//p')

######## Assumed packages
( dpkg -l | grep '^.i' | grep hostapd > /dev/null) || \
    apt_get_install hostapd

( dpkg -l | grep '^.i' | grep dnsmasq > /dev/null) || \
    apt_get_install dnsmasq

( dpkg -l | grep '^.i' | grep nginx > /dev/null) || \
    apt_get_install nginx

######## /etc/network/interfaces
if [ ! -f /etc/network/derbynet ] ; then
    ifdown wlan0 2>/dev/null
    cat >/etc/network/derbynet <<EOF
iface wlan0 inet static
  address 10.10.10.1
  netmask 255.255.255.0
EOF
    ifup wlan0
fi

grep /etc/network/derbynet /etc/network/interfaces > /dev/null || \
    sed -i.pre-derbynet \
        -e '/iface wlan0/,/^\s*$/ s/^\(\s*[^ ]\)/#\1/' \
        -e '$ asource /etc/network/derbynet' \
        /etc/network/interfaces


######## /etc/hostapd/hostapd.conf

sed -i -e 's!#DAEMON_CONF=""!DAEMON_CONF="/etc/hostapd/hostapd.conf"!' /etc/default/hostapd

# TODO Unfortunately, channel scanning doesn't appear to work with the
# RPi3 and its wifi driver.
if [ -f /etc/hostapd/hostapd.conf -a ! -f /etc/hostapd/hostapd.conf.pre-derbynet ] ; then
    cp /etc/hostapd/hostapd.conf /etc/hostapd/hostapd.conf.pre-derbynet
fi

gunzip -dc /usr/share/doc/hostapd/examples/hostapd.conf.gz | \
    sed -e "s/^ssid=.*/ssid=derbynet-pi-$SERIAL/" -e 's/^channel=.*/channel=1/' \
	> /etc/hostapd/hostapd.conf

service hostapd restart

######## dnsmasq

cat <<EOF > /etc/dnsmasq.d/derbynet-portal.conf

# DHCP
dhcp-authoritative
interface=wlan0
no-poll
dhcp-range=10.10.10.10,10.10.10.200,255.255.255.0,12h
dhcp-option=3,10.10.10.1
log-dhcp

# DNS
no-hosts
address=/#/10.10.10.1
EOF

grep derbynet /etc/dnsmasq.conf > /dev/null || \
    sed -i.pre-derbynet \
        -e '$ aconf-file=/etc/dnsmasq.d/derbynet-portal.conf' \
	/etc/dnsmasq.conf

service dnsmasq restart

######## Disable vanilla default site
rm -f /etc/nginx/sites-enabled/default 2>/dev/null

######## redirects for all other sites


# TODO Clients coming in to 10.10.10.1 get this server clause,
# even if they're already looking for derby.net.

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
