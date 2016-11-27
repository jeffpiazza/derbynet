#! /bin/sh

# Executes PHP code directly from the command line, rather than sending to a web server.

pushd "$(dirname "$0")" > /dev/null
DIR="$(pwd)"
UPDIR="$(dirname "$DIR")"
popd > /dev/null

php -f "$DIR/test-face-detection.php" "$UPDIR/website/inc/facedetection.inc" "$DIR/data/headshots"
