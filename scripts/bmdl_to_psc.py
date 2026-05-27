#!/usr/bin/env python3
"""Convert a Bluepoint Open3d .bmdl file to a MODL v2 .psc file.

Usage:
  python bmdl_to_psc.py <input.bmdl> [output.psc]

The converter:
  - Merges all Submesh elements into a single mesh chunk.
  - Reconciles split position/UV index arrays into a unified vertex list.
  - Normalises the mesh to fit inside a 2-unit bounding box centred at origin.
  - Writes positions (Float3) + white vertex colours (UByte4N) + UVs (Float2).
  - Outputs MODL v2 binary identical in layout to gen_modl.py output.
"""

import base64, struct, sys, os
import xml.etree.ElementTree as ET

# ---------------------------------------------------------------------------
# MODL v2 helpers (little-endian blob, big-endian stream fields)
# ---------------------------------------------------------------------------

def be_u32(v): return struct.pack('>I', v)
def be_i32(v): return struct.pack('>i', v)
def be_i16(v): return struct.pack('>h', v)
def be_f32(v): return struct.pack('>f', v)
def u8(v):     return struct.pack('B', v)
def le_u32(v): return struct.pack('<I', v)
def le_f32(v): return struct.pack('<f', v)

kVDT_Float2  = 0
kVDT_Float3  = 1
kVDT_UByte4N = 3
kFlags_IsOffset = 2
STREAM_SIZE = 16

POS0 = int.from_bytes(b'POS0', 'big')
COL0 = int.from_bytes(b'COL0', 'big')
TEX0 = int.from_bytes(b'TEX0', 'big')

def make_stream_le(stream_id, vdt_type, offset, size):
    return (le_u32(stream_id)
            + struct.pack('<H', vdt_type)
            + struct.pack('<H', kFlags_IsOffset)
            + le_u32(offset)
            + le_u32(size))


def build_vertex_blob(positions, colors_rgba_le, uvs):
    n           = len(positions)
    num_streams = 3
    hdr_size    = 4 + num_streams * STREAM_SIZE
    pos_size    = n * 12
    col_size    = n * 4
    uv_size     = n * 8
    pos_off     = hdr_size
    col_off     = hdr_size + pos_size
    uv_off      = hdr_size + pos_size + col_size

    blob  = le_u32(num_streams)
    blob += make_stream_le(POS0, kVDT_Float3,  pos_off, pos_size)
    blob += make_stream_le(COL0, kVDT_UByte4N, col_off, col_size)
    blob += make_stream_le(TEX0, kVDT_Float2,  uv_off,  uv_size)
    for (x, y, z) in positions:
        blob += le_f32(x) + le_f32(y) + le_f32(z)
    for c in colors_rgba_le:
        blob += le_u32(c)
    for (u, v) in uvs:
        blob += le_f32(u) + le_f32(v)
    return blob


def build_psc(positions, colors, uvs, indices, bounds_min, bounds_max):
    blob = build_vertex_blob(positions, colors, uvs)

    out  = b'MODL'
    out += be_i32(2)
    out += be_u32(len(blob))
    out += blob
    out += be_u32(len(indices))
    for i in indices:
        out += be_u32(i)
    out += be_i32(0)       # material count
    out += be_u32(1)       # chunk count
    # CMeshChunk
    for v in bounds_min: out += be_f32(v)
    for v in bounds_max: out += be_f32(v)
    out += be_i16(-1)      # materialIndex
    out += u8(0)           # primitiveType = TriangleList
    out += be_u32(0)       # vertexBufferOffset
    out += be_u32(len(positions))
    out += be_u32(0)       # indexBufferOffset
    out += be_u32(len(indices))
    out += be_u32(0)       # jointMap count
    out += be_u32(0x00000000)  # mVertColorMin
    out += be_u32(0xFFFFFFFF)  # mVertColorMax
    out += be_u32(0)       # mAdditionalFlags
    out += be_i32(0)       # mUnitIdx
    out += be_i32(0)       # mPacketIdx
    out += be_i32(0)       # jointCount
    return out

# ---------------------------------------------------------------------------
# bmdl parsing
# ---------------------------------------------------------------------------

def decode_array(da):
    """Decode a <DataArray> element → list of tuples of floats."""
    count  = int(da.get('count'))
    typ    = da.get('type')
    nf     = {'Vector4': 4, 'Vector3': 3, 'Vector2': 2}[typ]
    raw    = base64.b64decode(da.text.strip())
    stride = nf * 4
    return [struct.unpack_from(f'{nf}f', raw, i * stride) for i in range(count)]


def convert(bmdl_path, psc_path):
    tree = ET.parse(bmdl_path)
    root = tree.getroot()
    node = root.find('Node')

    # Decode vertex streams
    arrays = {}
    for da in node.findall('DataArray'):
        arrays[da.get('name')] = decode_array(da)

    pts  = arrays.get('Points',  [])   # float4 (x,y,z,1)
    uvs0 = arrays.get('UV0',    [])    # float4 (u,v,0,0)

    # Merge all submeshes into one triangle list.
    # Each submesh has a "Points;Normals" IndexArray and a "UV0" IndexArray.
    # These may differ, so we build a unified vertex table keyed on (pos_idx, uv_idx).
    out_positions = []
    out_uvs       = []
    out_colors    = []
    out_indices   = []
    vertex_map    = {}   # (pos_idx, uv_idx) → new_vertex_idx

    for sub in node.findall('Submesh'):
        ia_pn = None
        ia_uv = None
        for ia in sub.findall('IndexArray'):
            inp = ia.get('input', '')
            idxs = list(map(int, ia.text.split()))
            if 'Points' in inp:
                ia_pn = idxs
            elif 'UV0' in inp:
                ia_uv = idxs

        if ia_pn is None:
            continue
        if ia_uv is None:
            ia_uv = ia_pn  # fall back to same indices

        count = len(ia_pn)
        if len(ia_uv) != count:
            ia_uv = ia_pn  # mismatched lengths — fall back

        for i in range(count):
            pi = ia_pn[i]
            ui = ia_uv[i]
            key = (pi, ui)
            if key not in vertex_map:
                vertex_map[key] = len(out_positions)
                p = pts[pi]
                u = uvs0[ui] if ui < len(uvs0) else (0.0, 0.0, 0.0, 0.0)
                out_positions.append((p[0], p[1], p[2]))
                out_uvs.append((u[0], u[1]))
                out_colors.append(0xFFFFFFFF)  # white — texture provides colour
            out_indices.append(vertex_map[key])

    if not out_positions:
        print('ERROR: no geometry found in', bmdl_path, file=sys.stderr)
        sys.exit(1)

    # Normalise: centre + scale so the largest dimension = 2 units.
    xs = [p[0] for p in out_positions]
    ys = [p[1] for p in out_positions]
    zs = [p[2] for p in out_positions]
    cx = (min(xs) + max(xs)) * 0.5
    cy = (min(ys) + max(ys)) * 0.5
    cz = (min(zs) + max(zs)) * 0.5
    ext = max(max(xs)-min(xs), max(ys)-min(ys), max(zs)-min(zs))
    scale = 2.0 / ext if ext > 0.0 else 1.0

    out_positions = [((x - cx) * scale,
                      (y - cy) * scale,
                      (z - cz) * scale) for (x, y, z) in out_positions]

    xs = [p[0] for p in out_positions]
    ys = [p[1] for p in out_positions]
    zs = [p[2] for p in out_positions]
    bounds_min = (min(xs), min(ys), min(zs))
    bounds_max = (max(xs), max(ys), max(zs))

    data = build_psc(out_positions, out_colors, out_uvs, out_indices,
                     bounds_min, bounds_max)

    with open(psc_path, 'wb') as f:
        f.write(data)

    print(f'Converted {os.path.basename(bmdl_path)}'
          f' → {psc_path}'
          f'  ({len(out_positions)} verts, {len(out_indices)//3} tris,'
          f' {len(data)} bytes)')


# ---------------------------------------------------------------------------

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)

    bmdl = sys.argv[1]
    if len(sys.argv) >= 3:
        psc = sys.argv[2]
    else:
        psc = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                           'disc_data',
                           os.path.splitext(os.path.basename(bmdl))[0] + '.psc')

    convert(bmdl, psc)
