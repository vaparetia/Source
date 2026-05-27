#!/usr/bin/env python3
"""Extract crate texture from TGA flatlist and convert to DC-twiddled RGB565.

Reads:  GameData/textures/flatlist/cbx_a.bmp.tga  (256x256 BGRA, PS2-style alpha)
Writes: scripts/disc_data/crate.tex               (DC twiddled RGB565, 131072 bytes)

Dreamcast PVR hardware requires the 'twiddled' (Z-order / Morton-code) layout
when pvr_poly_cxt_txr is called with PVR_TXRFMT_TWIDDLED.  Uploading linear
data with that flag set produces a visibly scrambled texture.
"""

import struct, os, sys

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC_TGA   = os.path.join(REPO_ROOT, 'GameData', 'textures', 'flatlist', 'cbx_a.bmp.tga')
OUT_TEX   = os.path.join(REPO_ROOT, 'scripts', 'disc_data', 'crate.tex')


# ---------------------------------------------------------------------------
# Morton-code (twiddle) address for Dreamcast PVR.
# For square W×W textures this interleaves x and y bits.

def twiddle_idx(x, y, W, H):
    # DC PVR twiddle: Y bits occupy even bit positions (0,2,4,...),
    # X bits occupy odd positions (1,3,5,...).
    n_bits = min(W, H).bit_length() - 1
    t = 0
    for i in range(n_bits):
        t |= ((y >> i) & 1) << (2 * i)
        t |= ((x >> i) & 1) << (2 * i + 1)
    if W > H:
        t |= (x >> n_bits) << (2 * n_bits)
    elif H > W:
        t |= (y >> n_bits) << (2 * n_bits)
    return t


def twiddle_rgb565(linear_rgb565, W, H):
    """Re-order flat RGB565 array into DC twiddled layout."""
    out = [0] * (W * H)
    for y in range(H):
        for x in range(W):
            out[twiddle_idx(x, y, W, H)] = linear_rgb565[y * W + x]
    return out


# ---------------------------------------------------------------------------
# TGA reader (type 2 uncompressed only).

def read_tga(path):
    with open(path, 'rb') as f:
        data = f.read()

    id_len   = data[0]
    img_type = data[2]
    width,  height = struct.unpack_from('<HH', data, 12)
    bpp    = data[16]
    img_desc = data[17]
    origin_top = bool(img_desc & 0x20)  # bit 5 = top-left origin

    if img_type != 2:
        raise ValueError(f'Unsupported TGA type {img_type} (expected 2 = uncompressed RGB)')
    if bpp not in (24, 32):
        raise ValueError(f'Unsupported BPP {bpp}')

    pix_start = 18 + id_len
    raw = data[pix_start: pix_start + width * height * (bpp // 8)]

    pixels = []  # list of (r, g, b)
    stride = bpp // 8
    for i in range(width * height):
        off = i * stride
        b, g, r = raw[off], raw[off + 1], raw[off + 2]
        pixels.append((r, g, b))

    # TGA stores bottom-row first unless origin_top is set.
    if not origin_top:
        rows = [pixels[y * width:(y + 1) * width] for y in range(height)]
        rows.reverse()
        pixels = [px for row in rows for px in row]

    return width, height, pixels


# ---------------------------------------------------------------------------

def bgr_to_rgb565(r, g, b):
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)


def main():
    if not os.path.exists(SRC_TGA):
        print(f'ERROR: source not found: {SRC_TGA}', file=sys.stderr)
        sys.exit(1)

    W, H, pixels = read_tga(SRC_TGA)
    print(f'Read {W}x{H} TGA ({len(pixels)} pixels)')

    # Convert to linear RGB565 — pvr_txr_load_ex(PVR_TXRLOAD_16BPP) twiddled
    # the data itself on upload, so we must NOT pre-twiddle here.
    linear565 = [bgr_to_rgb565(r, g, b) for (r, g, b) in pixels]

    out_bytes = struct.pack(f'<{len(linear565)}H', *linear565)

    os.makedirs(os.path.dirname(OUT_TEX), exist_ok=True)
    with open(OUT_TEX, 'wb') as f:
        f.write(out_bytes)

    print(f'Wrote {len(out_bytes)} bytes → {OUT_TEX}')
    print(f'Texture: {W}x{H} RGB565 linear (pvr_txr_load_ex twiddled on upload)')


if __name__ == '__main__':
    main()
