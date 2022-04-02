#!/usr/bin/env python

import struct
import sys

scanner_device = sys.argv[1]

# Values taken from include/uapi/linux/input-event-codes.h
keys = {
2: '1', 3: '2', 4: '3', 5: '4', 6: '5', 7: '6', 8: '7', 9: '8', 10: '9', 11: '0', \
12: '-', 13: '=', \
16: 'q', 17: 'w', 18: 'e', 19: 'r', 20: 't', 21: 'y', 22: 'u', 23: 'i', 24: 'o', 25: 'p', \
26: '[', 27: ']', \
30: 'a', 31: 's', 32: 'd', 33: 'f', 34: 'g', 35: 'h', 36: 'j', 37: 'k', 38: 'l', 39: ';', \
40: '\'', 43: '\\', \
44: 'z', 45: 'x', 46: 'c', 47: 'v', 48: 'b', 49: 'n', 50: 'm', 51: ',', 52: '.', 53: '/', \
57: ' ' }

shift_keys = {
2: '!', 3: '@', 4: '#', 5: '$', 6: '%', 7: '^', 8: '&', 9: '*', 10: '(', 11: ')', \
12: '_', 13: '+', \
16: 'Q', 17: 'W', 18: 'E', 19: 'R', 20: 'T', 21: 'Y', 22: 'U', 23: 'I', 24: 'O', 25: 'P', \
26: '{', 27: '}', \
30: 'A', 31: 'S', 32: 'D', 33: 'F', 34: 'G', 35: 'H', 36: 'J', 37: 'K', 38: 'L', 39: ';', \
40: '\"', 43: '|', \
44: 'Z', 45: 'X', 46: 'C', 47: 'V', 48: 'B', 49: 'N', 50: 'M', 51: '<', 52: '>', 53: '?', \
57: ' ' }

KEY_ENTER = 28
KEY_LEFTSHIFT = 42
KEY_RIGHTSHIFT = 54

EV_VALUE_KEY_RELEASED = 0
EV_VALUE_KEY_PRESSED = 1
EV_VALUE_KEY_AUTOREPEAT = 2

EV_KEY = 1
# EV_SYN = 0
# EV_MSC = 4

# 4IHHI on 64-bit machines; each of the other INEV_ indices would increase by 2
INEV_STRUCT = '2IHHI'
# Offsets in the input_event struct
#INEV_XX0 = 0
#INEV_XX1 = 1
INEV_TYPE = 2
INEV_CODE = 3
INEV_VALUE = 4

ss = ""
with open(scanner_device, 'rb') as fp:
    shift = False
    done = False
    while not done:
        buffer = fp.read(struct.calcsize(INEV_STRUCT))
        ev = struct.unpack(INEV_STRUCT, buffer)

        if ev[INEV_TYPE] != EV_KEY:
            continue

        is_keypress = ev[INEV_VALUE] == EV_VALUE_KEY_PRESSED or \
                      ev[INEV_VALUE] == EV_VALUE_KEY_AUTOREPEAT
        # print ev
        if ev[INEV_CODE] == KEY_LEFTSHIFT or ev[INEV_CODE] == KEY_RIGHTSHIFT:
            shift = is_keypress
        elif is_keypress:
            if ev[INEV_CODE] == KEY_ENTER:
                done = True
            elif shift and ev[INEV_CODE] in shift_keys:
                ss += shift_keys[ev[INEV_CODE]]
            elif ev[INEV_CODE] in keys:
                ss += keys[ev[INEV_CODE]]

print(ss)
