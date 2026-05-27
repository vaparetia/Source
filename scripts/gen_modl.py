#!/usr/bin/env python3
"""Generate MODL v2 mesh binaries for Dreamcast testing.

Binary layout:
  - Stream API fields (ReadUint32/ReadReal32 etc.) are big-endian.
  - CVertexArray blob (ReadAsMemoryStream) is native LE.

Usage:
  python gen_modl.py [cube|triangle]   (default: both)
"""

import struct, sys, os

def be_u32(v): return struct.pack('>I', v)
def be_i32(v): return struct.pack('>i', v)
def be_i16(v): return struct.pack('>h', v)
def be_f32(v): return struct.pack('>f', v)
def u8(v):     return struct.pack('B', v)
def le_u32(v): return struct.pack('<I', v)
def le_i32(v): return struct.pack('<i', v)
def le_f32(v): return struct.pack('<f', v)

kVDT_Float2  = 0
kVDT_Float3  = 1
kVDT_UByte4N = 3
kFlags_IsOffset = 2

POS0 = int.from_bytes(b'POS0', 'big')
COL0 = int.from_bytes(b'COL0', 'big')
TEX0 = int.from_bytes(b'TEX0', 'big')
STREAM_SIZE = 16  # uint32 id + uint16 type + uint16 flags + uint32 data + uint32 size

def make_stream_le(stream_id, vdt_type, offset_from_blob, data_size):
    return (le_u32(stream_id) + struct.pack('<H', vdt_type) + struct.pack('<H', kFlags_IsOffset)
            + le_u32(offset_from_blob) + le_u32(data_size))

def build_vertex_blob(positions, colors_rgba_le, uvs=None):
    """Build CVertexArray blob (LE) for Float3 positions + UByte4N colours + optional Float2 UVs."""
    num_verts   = len(positions)
    num_streams = 3 if uvs else 2
    header_size = 4 + num_streams * STREAM_SIZE
    pos_size    = num_verts * 12
    col_size    = num_verts * 4
    uv_size     = num_verts * 8 if uvs else 0
    blob_size   = header_size + pos_size + col_size + uv_size
    pos_offset  = header_size
    col_offset  = header_size + pos_size
    uv_offset   = header_size + pos_size + col_size

    blob  = le_u32(num_streams)
    blob += make_stream_le(POS0, kVDT_Float3,  pos_offset, pos_size)
    blob += make_stream_le(COL0, kVDT_UByte4N, col_offset, col_size)
    if uvs:
        blob += make_stream_le(TEX0, kVDT_Float2, uv_offset, uv_size)
    for (x, y, z) in positions:
        blob += le_f32(x) + le_f32(y) + le_f32(z)
    for c in colors_rgba_le:
        blob += le_u32(c)
    if uvs:
        for (u, v) in uvs:
            blob += le_f32(u) + le_f32(v)
    assert len(blob) == blob_size
    return blob

def build_modl(positions, colors_rgba_le, indices, bounds_min, bounds_max,
               primitive_type=0, uvs=None):
    """
    Build a complete MODL v2 binary.
    primitive_type: 0=TriangleList, 1=TriangleStrip, 2=TriangleFan
    indices: list of int (uint32, will be written BE)
    uvs: optional list of (u,v) float pairs, one per vertex
    """
    blob = build_vertex_blob(positions, colors_rgba_le, uvs)

    out  = b'MODL'
    out += be_i32(2)           # version = 2
    out += be_u32(len(blob))
    out += blob
    # CIndexArray: count then uint32 indices
    out += be_u32(len(indices))
    for idx in indices:
        out += be_u32(idx)
    out += be_i32(0)           # material count
    out += be_u32(1)           # mesh chunk count
    # CMeshChunk:
    for v in bounds_min: out += be_f32(v)
    for v in bounds_max: out += be_f32(v)
    out += be_i16(-1)          # materialIndex
    out += u8(primitive_type)  # primitiveType
    out += be_u32(0)           # vertexBufferOffset
    out += be_u32(len(positions))  # vertexCount
    out += be_u32(0)           # indexBufferOffset
    out += be_u32(len(indices))    # indicesCount
    out += be_u32(0)           # jointMap count
    out += be_u32(0x00000000)  # mVertColorMin
    out += be_u32(0xFFFFFFFF)  # mVertColorMax
    out += be_u32(0)           # mAdditionalFlags
    out += be_i32(0)           # mUnitIdx
    out += be_i32(0)           # mPacketIdx
    out += be_i32(0)           # jointCount
    return out

# ---------------------------------------------------------------------------
# Triangle (NDC coords, for quick sanity-check without a camera)
# ---------------------------------------------------------------------------

def make_triangle():
    positions = [
        (-0.5, -0.5,  0.5),
        ( 0.5, -0.5,  0.5),
        ( 0.0,  0.5,  0.5),
    ]
    colors = [0xFF0000FF, 0xFF00FF00, 0xFFFF0000]  # R G B (LE RGBA→ARGB decode)
    uvs    = [(0.0, 1.0), (1.0, 1.0), (0.5, 0.0)]
    indices = [0, 1, 2]
    return build_modl(positions, colors, indices,
                      (-0.5,-0.5,0.5), (0.5,0.5,0.5), uvs=uvs)

# ---------------------------------------------------------------------------
# Cube  (unit cube centred at origin, side length 1)
#
# Placed at z = -3 in world space so it sits in front of the identity camera
# after the SetCameraMatrix Scale(1,1,-1) view flip.
# With perspective (FOV=60, aspect=4/3, near=0.1, far=100) it maps neatly
# into view.
#
# 24 vertices (4 per face × 6 faces) so each face gets distinct colours
# without index-dependent lighting tricks.
# ---------------------------------------------------------------------------

def make_cube():
    # Cube centred at origin in model space, half-extent 0.8.
    # DCMain.cpp places it at z=-3 via the model matrix.
    cx, cy, cz = 0.0, 0.0, 0.0
    h = 0.8

    # Face colours (one per face): RGBA stored as LE uint32
    # Engine decode: argb = (c[3]<<24)|(c[0]<<16)|(c[1]<<8)|c[2]
    # LE uint32 0xAABBGGRR → bytes [RR,GG,BB,AA] → argb = (AA<<24)|(RR<<16)|(GG<<8)|BB
    # So to get argb=0xFFRRGGBB, store LE uint32 = 0xFFBBGGRR
    RED    = 0xFF0000FF  # → argb 0xFFFF0000
    GREEN  = 0xFF00FF00  # → argb 0xFF00FF00
    BLUE   = 0xFFFF0000  # → argb 0xFF0000FF
    YELLOW = 0xFF00FFFF  # → argb 0xFFFFFF00
    CYAN   = 0xFFFFFF00  # → argb 0xFF00FFFF
    WHITE  = 0xFFFFFFFF  # → argb 0xFFFFFFFF

    xm, xp = cx - h, cx + h
    ym, yp = cy - h, cy + h
    zm, zp = cz - h, cz + h

    # Each face: 4 verts (CCW from outside), 2 triangles = 6 indices
    # Winding: engine uses CULLING_NONE so order doesn't matter for now.
    faces = [
        # +Z front
        ([(xm,ym,zp),(xp,ym,zp),(xp,yp,zp),(xm,yp,zp)], RED),
        # -Z back
        ([(xp,ym,zm),(xm,ym,zm),(xm,yp,zm),(xp,yp,zm)], GREEN),
        # +Y top
        ([(xm,yp,zm),(xp,yp,zm),(xp,yp,zp),(xm,yp,zp)], BLUE),
        # -Y bottom
        ([(xm,ym,zp),(xp,ym,zp),(xp,ym,zm),(xm,ym,zm)], YELLOW),
        # +X right
        ([(xp,ym,zp),(xp,ym,zm),(xp,yp,zm),(xp,yp,zp)], CYAN),
        # -X left
        ([(xm,ym,zm),(xm,ym,zp),(xm,yp,zp),(xm,yp,zm)], WHITE),
    ]

    positions = []
    colors    = []
    uvs       = []
    indices   = []

    # Per-face quad UV layout: CCW from bottom-left
    face_uvs = [(0.0, 1.0), (1.0, 1.0), (1.0, 0.0), (0.0, 0.0)]

    for face_verts, color in faces:
        base = len(positions)
        for i, v in enumerate(face_verts):
            positions.append(v)
            colors.append(color)
            uvs.append(face_uvs[i])
        # Two triangles per quad face (indices relative to base)
        indices += [base+0, base+1, base+2, base+0, base+2, base+3]

    return build_modl(positions, colors, indices,
                      (xm, ym, zm), (xp, yp, zp), uvs=uvs)

# ---------------------------------------------------------------------------

def write(name, data):
    dest = os.path.join(os.path.dirname(__file__), 'disc_data', name)
    os.makedirs(os.path.dirname(dest), exist_ok=True)
    with open(dest, 'wb') as f:
        f.write(data)
    print(f'Written {len(data)} bytes → {dest}')

if __name__ == '__main__':
    targets = sys.argv[1:] or ['triangle', 'cube']
    if 'triangle' in targets:
        write('test.psc', make_triangle())
    if 'cube' in targets:
        write('cube.psc', make_cube())
