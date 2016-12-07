#! /bin/sh

mkdir -m 777 /Library/WebServer/Documents/derbynet/local

mkdir -m 775 "$HOME/Public/DerbyNet"
chown $USER _www "$HOME/Public/DerbyNet"

cat <<EOF > /Library/WebServer/Documents/derbynet/local/default-file-path.inc
<?php
\$default_file_path = "$HOME/Public/DerbyNet";
?>
EOF
