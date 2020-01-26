#!/usr/bin/env python

import base64
import zlib
import json

import sys

blueprint_string = sys.argv[1]

assert(blueprint_string[0] == '0')

bs_vs = blueprint_string[1:]
bs_b64 = bytes(bs_vs, 'ascii')
bs_z = base64.decodebytes(bs_b64)
bs_j = zlib.decompress(bs_z)
bp = json.loads(bs_j)

bs_j_2 = bytes(json.dumps(bp, separators=(',', ':')), 'ascii')
bs_z_2 = zlib.compress(bs_j_2, level=9)
bs_b64_2 = bytes(base64.encodebytes(bs_z_2).decode('ascii').replace('\n', ''), 'ascii')
bs_vs_2 = bs_b64_2.decode('ascii')
blueprint_string_2 = '0' + bs_vs_2

assert(bs_j == bs_j_2)
assert(bs_z == bs_z_2)
assert(bs_b64 == bs_b64_2)
assert(bs_vs == bs_vs_2)
assert(blueprint_string == blueprint_string_2)

print(json.dumps(bp, indent=4))
