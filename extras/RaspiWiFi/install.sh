#!/bin/bash

############################################################
# Shell script to install DerbyNet Timer on a raspberry pi.
# We also install RaspiWiFi to make a headless timer
# easier.
############################################################

if [ $USER != "root" ] ; then
    echo Please run with sudo
    exit 1
fi

echo "This operation will install the DerbyNet Timer, configure systemd, and launch RaspiWiFi setup.

Press Ctrl+C to cancel."
read

# Install DerbyNet Timer
wget -q -O- https://jeffpiazza.org/derbynet/debian/jeffpiazza_derbynet.gpg > /usr/share/keyrings/derbynet-archive-keyring.gpg
echo "deb [arch=all signed-by=/usr/share/keyrings/derbynet-archive-keyring.gpg] " \
" https://jeffpiazza.org/derbynet/debian stable main" > /etc/apt/sources.list.d/derbynet.list
apt update
apt install -y derbynet-timer

# Setup the service
useradd -md /var/lib/derbynet -G tty,dialout,plugdev derbynet
cat <<HEREDOC > /usr/lib/systemd/system/derbynet-timer.service
[Unit]
Description=DerbyNet Timer
#Documentation=
Wants=network-online.target
After=network-online.target

[Service]
#EnvironmentFile=
User=derbynet
Group=derbynet
Type=simple
Restart=on-failure
WorkingDirectory=/var/lib/derbynet
RuntimeDirectory=/var/lib/derbynet
RuntimeDirectoryMode=0777
ExecStart=/usr/share/derbynet/scripts/derby-timer.sh

[Install]
WantedBy=multi-user.target
HEREDOC
systemctl daemon-reload
systemctl enable derbynet-timer.service
systemctl start derbynet-timer.service

# Install RaspiWiFi
apt install -y git python3
git clone https://github.com/jasbur/RaspiWiFi.git
cd RaspiWiFi/
python3 initial_setup.py
cd ../
