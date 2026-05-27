#!/usr/bin/env python3
"""Generate a 64x64 RGB565 checkerboard texture for Dreamcast PVR.

Output is a raw linear RGB565 file (2 bytes/pixel, little-endian).
pvr_txr_load_ex with PVR_TXRLOAD_16BPP converts to twiddled format on upload.
"""

import struct, os

W, H  = 64, 64
TILE  = 8        # checker square size in pixels
WHITE = 0xFFFF   # RGB565 white
GREY  = 0x39E7   # RGB565 ~(55,57,55) dark grey

data = bytearray()
for y in range(H):
    for x in range(W):
        val = WHITE if ((x // TILE) + (y // TILE)) % 2 == 0 else GREY
        data += struct.pack('<H', val)

dest = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'disc_data', 'cube.tex')
os.makedirs(os.path.dirname(dest), exist_ok=True)
with open(dest, 'wb') as f:
    f.write(data)
print(f'Written {len(data)} bytes → {dest}')
