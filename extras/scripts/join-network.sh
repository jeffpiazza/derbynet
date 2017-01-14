#! /bin/sh

NETWORK_INDEX=`wpa_cli add_network | tail -n +2`
echo NETWORK_INDEX is $NETWORK_INDEX
wpa_cli set_network $NETWORK_INDEX ssid \"YourNetworkName\"
wpa_cli set_network $NETWORK_INDEX psk \"TheWifiPassword\"

# For some networks, it's useful to limit the correct protocol if several are
# available from the access point.  The default is to try either WPA or WPA2.
#
# wpa_cli set_network $NETWORK_INDEX proto WPA2

wpa_cli enable_network $NETWORK_INDEX

