#!/usr/bin/env python3
"""Generate a minimal MODL v2 mesh binary for Dreamcast testing.

Binary layout:
  - Stream API fields (ReadUint32/ReadReal32 etc.) are big-endian (NEndian::Swap swaps on LE).
  - CVertexArray blob (ReadAsMemoryStream) is native LE, matching both x86 generator and SH4 target.

Triangle in NDC (identity MVP → identity camera + identity projection):
  v0: (-0.5, -0.5, 0.5) → screen (160, 360)  — red
  v1: ( 0.5, -0.5, 0.5) → screen (480, 360)  — green
  v2: ( 0.0,  0.5, 0.5) → screen (320, 120)  — blue
"""

import struct, sys, os

def be_u32(v): return struct.pack('>I', v)
def be_i32(v): return struct.pack('>i', v)
def be_i16(v): return struct.pack('>h', v)
def be_f32(v): return struct.pack('>f', v)
def u8(v):     return struct.pack('B', v)

def le_u32(v): return struct.pack('<I', v)
def le_u16(v): return struct.pack('<H', v)
def le_f32(v): return struct.pack('<f', v)

# ---------------------------------------------------------------------------
# CVertexArray blob (little-endian layout for direct memory cast on SH4)
# ---------------------------------------------------------------------------

kVDT_Float3  = 1
kVDT_UByte4N = 3
kFlags_IsOffset = 2  # mData field contains offset from blob start, not a ptr

# Stream IDs as C++ multi-char literals (big-endian interpretation = GCC value)
POS0 = int.from_bytes(b'POS0', 'big')  # 0x504F5330
COL0 = int.from_bytes(b'COL0', 'big')  # 0x434F4C30

NUM_STREAMS = 2
NUM_VERTS   = 3

# struct Stream { uint32 mId; uint16 mType; uint16 mFlags; uint32 mData; uint32 mSize; }
STREAM_SIZE = 4 + 2 + 2 + 4 + 4  # = 16 bytes

# Blob layout:
#   +0  : uint32 mStreamCount
#   +4  : Stream[0] POS0  (16 bytes)
#   +20 : Stream[1] COL0  (16 bytes)
#   +36 : POS0 data  — 3 × Float3 = 36 bytes
#   +72 : COL0 data  — 3 × UByte4N = 12 bytes
#   total = 84 bytes

HEADER_SIZE  = 4 + NUM_STREAMS * STREAM_SIZE  # 36
POS_SIZE     = NUM_VERTS * 12                  # 36
COL_SIZE     = NUM_VERTS * 4                   # 12
BLOB_SIZE    = HEADER_SIZE + POS_SIZE + COL_SIZE  # 84

POS_OFFSET   = HEADER_SIZE           # 36
COL_OFFSET   = HEADER_SIZE + POS_SIZE  # 72

def make_stream_le(stream_id, vdt_type, offset_from_blob, data_size):
    return (le_u32(stream_id) + le_u16(vdt_type) + le_u16(kFlags_IsOffset)
            + le_u32(offset_from_blob) + le_u32(data_size))

# Vertex positions (LE floats — raw memory, no byte-swap on load)
positions = [
    (-0.5, -0.5,  0.5),  # v0
    ( 0.5, -0.5,  0.5),  # v1
    ( 0.0,  0.5,  0.5),  # v2
]

# Vertex colors — RGBA bytes stored as LE uint32: R|G<<8|B<<16|A<<24
# Engine decodes: argb = (c[3]<<24)|(c[0]<<16)|(c[1]<<8)|c[2]
# So for red (R=255,G=0,B=0,A=255): LE uint32 = 0xFF0000FF, bytes [0xFF,0x00,0x00,0xFF]
# argb = (0xFF<<24)|(0xFF<<16)|(0<<8)|0 = 0xFFFF0000 (opaque red) ✓
colors_rgba_le = [
    0xFF0000FF,  # v0: red
    0xFF00FF00,  # v1: green
    0xFFFF0000,  # v2: blue
]

blob  = le_u32(NUM_STREAMS)
blob += make_stream_le(POS0, kVDT_Float3,  POS_OFFSET, POS_SIZE)
blob += make_stream_le(COL0, kVDT_UByte4N, COL_OFFSET, COL_SIZE)
for (x, y, z) in positions:
    blob += le_f32(x) + le_f32(y) + le_f32(z)
for c in colors_rgba_le:
    blob += le_u32(c)

assert len(blob) == BLOB_SIZE, f"blob size mismatch: {len(blob)} != {BLOB_SIZE}"

# ---------------------------------------------------------------------------
# Build the complete MODL v2 binary
# ---------------------------------------------------------------------------

out = b''

# Header
out += b'MODL'       # uint32 BE magic ('MODL' == 0x4D4F444C)
out += be_i32(2)     # version = 2

# CVertexArray::Load: reads uint32 size, then ReadAsMemoryStream(size)
out += be_u32(BLOB_SIZE)  # blob size (stream-style, big-endian)
out += blob               # raw struct bytes (little-endian layout)

# CIndexArray(stream) → vector_s<uint32>(stream): count then N×uint32
out += be_u32(3)     # index count
out += be_u32(0)     # index 0
out += be_u32(1)     # index 1
out += be_u32(2)     # index 2

# material count (int32 BE, loop body is empty — no materials built)
out += be_i32(0)

# mesh chunk count (uint32 BE)
out += be_u32(1)

# CMeshChunk(stream, version=2):
#   CAABox(stream) = mMin CVector3 + mMax CVector3 (each 3 × real32 BE)
out += be_f32(-0.5) + be_f32(-0.5) + be_f32(0.5)  # mMin
out += be_f32( 0.5) + be_f32( 0.5) + be_f32(0.5)  # mMax
#   int16 materialIndex
out += be_i16(-1)
#   uint8 primitiveType (kPrimitive_TriangleList = 0)
out += u8(0)
#   uint32 vertexBufferOffset
out += be_u32(0)
#   uint32 vertexCount
out += be_u32(3)
#   uint32 indexBufferOffset
out += be_u32(0)
#   uint32 indicesCount
out += be_u32(3)
#   vector_s<uint32> jointMap (count = 0)
out += be_u32(0)
#   uint32 mVertColorMin
out += be_u32(0x00000000)
#   uint32 mVertColorMax
out += be_u32(0xFFFFFFFF)
#   uint32 mAdditionalFlags
out += be_u32(0)
#   int32 mUnitIdx
out += be_i32(0)
#   int32 mPacketIdx
out += be_i32(0)

# joint count (int32 BE)
out += be_i32(0)

# ---------------------------------------------------------------------------
dest = os.path.join(os.path.dirname(__file__), 'disc_data', 'test.psc')
os.makedirs(os.path.dirname(dest), exist_ok=True)
with open(dest, 'wb') as f:
    f.write(out)
print(f'Written {len(out)} bytes → {dest}')
