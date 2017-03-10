#! /bin/sh

JoinWithPassword() {
    NETWORK_INDEX=`wpa_cli add_network | tail -n +2`
    wpa_cli set_network $NETWORK_INDEX ssid \"$1\"
    wpa_cli set_network $NETWORK_INDEX psk \"$2\"
    # For some networks, it's useful to limit the correct protocol if several are
    # available from the access point.  The default is to try either WPA or WPA2.
    #
    # wpa_cli set_network $NETWORK_INDEX proto WPA2
    wpa_cli enable_network $NETWORK_INDEX
}

JoinOpenNetwork() {
    NETWORK_INDEX=`wpa_cli add_network | tail -n +2`
    wpa_cli set_network $NETWORK_INDEX ssid \"$1\"
    wpa_cli set_network $NETWORK_INDEX key_mgmt NONE
    wpa_cli enable_network $NETWORK_INDEX
}

# Uncomment one or the other (or both or neither); repeat as needed

# JoinWithPassword "YourNetworkName" "TheWifiPassword"
# JoinOpenNetwork "YourNetworkName"
