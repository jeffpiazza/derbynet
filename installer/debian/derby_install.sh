
# Install Raspbian Bullseye
# Install all updates
# Enable SSH from the Preferences, Raspberry Pi Configuration

# # Adding DerbyNet Apt Repo
sudo apt-get update
sudo apt-get install -y apt-transport-https
wget -q -O- https://jeffpiazza.org/derbynet/debian/jeffpiazza_derbynet.gpg | sudo tee /usr/share/keyrings/derbynet-archive-keyring.gpg > /dev/null
echo "deb [arch=all signed-by=/usr/share/keyrings/derbynet-archive-keyring.gpg] " " https://jeffpiazza.org/derbynet/debian stable main" | sudo tee /etc/apt/sources.list.d/derbynet.list > /dev/null
sudo apt-get update

# # Install
# Clean up older versions
sudo apt-get remove derbynet

# # New Version
sudo apt-get install derbynet-server
sudo apt-get remove apache2

# # Checks
# Folder /usr/share/derbynet will now exist

# # Timer
sudo apt-get install derbynet-timer
# sudo derby-timer.jar to get it started
# or maybe this: java -jar /usr/bin/derby-timer.jar -simulate-timer -lanes 4 to simulate the timer

# # Extras
sudo apt-get install derbynet-extras
