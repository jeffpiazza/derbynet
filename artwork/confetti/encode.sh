#! /bin/sh

# confetti.blend in Blender should be able to generate a sequence of PNG frames with alpha channel.
# These go in /tmp, but that might vary on other platforms.

PNG_DIR=/tmp
OUT_DIR=./website

# There doesn't seem to be one universal video format that's supported by all
# browsers and that supports alpha channel transparency.  So we generate the
# movie in two forms:

ffmpeg -i $PNG_DIR/%04d.png             $OUT_DIR/confetti.webm

# This command produces a .mov file using the ProRes codec, which is apparently the only
# one that supports alpha channel in the video.  Unfortunately, the resulting .mov file
# is nearly 40Mb.
ffmpeg -i $PNG_DIR/%04d.png -vcodec prores_ks -pix_fmt yuva444p10 $OUT_DIR/confetti.mov
