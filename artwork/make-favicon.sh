#! /bin/sh

INKSCAPE="/Applications/Inkscape.app/Contents/Resources/bin/inkscape"
IMAGEMAGICK="/opt/local/bin/convert"

# Bugs in this inkscape script cause inkscape-bin to run with the "working
# directory" set elsewhere, so relative paths on the command line don't work
# correctly.  This pushd/popd allows using pwd to produce the absolute path.
pushd "`dirname $0`" > /dev/null 2>&1
ARTWORK="`pwd`"
popd > /dev/null

TEMP="`mktemp -d`"

# At smaller sizes, use favicon.svg
for SIZE in 16 24 32 ; do
    "$INKSCAPE" -z -e "$TEMP/favicon-$SIZE.png" -h $SIZE -w $SIZE "$ARTWORK/favicon.svg"
done

# At larger sizes, switch to derbynet.svg
for SIZE in 48 64 ; do
    "$INKSCAPE" -z -e "$TEMP/favicon-$SIZE.png" -h $SIZE -w $SIZE "$ARTWORK/derbynet.svg"
done

"$IMAGEMAGICK" "$TEMP/favicon-32.png" "$TEMP/favicon-16.png" "$TEMP/favicon-48.png" "$TEMP/favicon-24.png" "$TEMP/favicon-64.png" "$ARTWORK/../website/favicon.ico"
