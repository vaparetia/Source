# 02. Hardware Comparison — PS2 vs. Dreamcast vs. Modern Reference

## Quick Reference Table

| Spec | PS2 (Original) | Dreamcast | Modern PC | Notes |
|------|---|---|---|---|
| **CPU** | Emotion Engine 300 MHz | SH-4 200 MHz | 3+ GHz | Dreamcast is ~0.67x PS2 speed |
| **RAM** | 32 MB | 16 MB | 16+ GB | Dreamcast requires tight memory management |
| **VRAM** | 4 MB | 8 MB | 2+ GB | Dreamcast actually has advantage here |
| **GPU** | Graphics Synthesizer | PowerVR Kyro II | Modern | Completely different architectures |
| **Polygon Budget** | ~10-15M/frame | ~3-5M/frame | 100M+ | Dreamcast needs 1/3 the polygons |
| **Texture Filtering** | BiLinear | Bilinear | Trilinear+ | Same approach works |
| **Audio** | 48 kHz, multi-channel | 48 kHz stereo | 48 kHz+ | Compatible |
| **Disc Capacity** | 700 MB (DVD) | 700 MB (GD-ROM) | N/A | Same, functionally |
| **Network** | None (native) | 100 Mbps Ethernet | Gig+ | Dreamcast can stream assets |

---

## Detailed CPU Comparison

### PS2: Emotion Engine (MIPS-based)

```
Architecture:    Custom MIPS64-based design
Clock Speed:     300 MHz
Vector Unit:     Custom Vector Unit (VU0/VU1)
Cache:           Good (32KB L1 per core)
Strength:        Custom vector instructions for game logic
Weakness:        Harder to optimize than standard ISA
```

**What This Means**: Developers wrote code specifically for MIPS. When porting to Dreamcast, you'll be recompiling C/C++ to SH-4, which will likely be slower.

### Dreamcast: Hitachi SH-4

```
Architecture:    Hitachi SH-4 (RISC)
Clock Speed:     200 MHz (33% slower than PS2)
Vector Unit:     SH-4 SIMD instructions (not as powerful)
Cache:           16 KB L1 (smaller than PS2)
Strength:        Standard ISA, GCC compiler support
Weakness:        No custom vector unit like PS2
```

**What This Means**: Dreamcast is ~33% slower on the same code. This is your biggest challenge.

### Real-World Impact

If PS2 MGS2 can render 60 FPS:
- Dreamcast running same code: ~40 FPS (theoretical)
- After optimization: ~20-30 FPS (realistic)
- With gameplay cuts: ~30 FPS (achievable)

**Conclusion**: You need CPU optimization or gameplay simplification. No way around it.

---

## GPU Architecture: The Fundamental Difference

### PS2: Graphics Synthesizer (Immediate Mode Renderer)

```
Rendering Model:    Immediate mode (process geometry as you send it)
Tile Size:          N/A (processes full framebuffer)
Bandwidth:          ~6.4 GB/s
Strengths:          Great for complex shaders, flexible rendering
Weaknesses:         Higher bandwidth requirements
```

**How It Works**: GPU processes triangles one at a time. Good for complex, varied geometry.

### Dreamcast: PowerVR Kyro II (Tile-Based Deferred Renderer)

```
Rendering Model:    Tile-based (divide screen into tiles, process each)
Tile Size:          32x32 pixels
Bandwidth:          ~1.4 GB/s (lower, more efficient)
Strengths:         Efficient memory usage, predictable performance
Weaknesses:        Requires specific optimization patterns
```

**How It Works**: Screen divided into 32x32 pixel tiles. GPU processes each tile separately. Dramatically different from immediate mode.

### What This Means for Your Port

**Challenge**: The rendering code you write must be compatible with tile-based deferred rendering.

**Solution Approach**:
1. **Forward Rendering**: Draw geometry → render to texture → compose on screen
2. **Avoid Reading Results Back**: Tile-based GPUs hate reading from framebuffer
3. **Batch Geometry**: Group objects by texture to minimize state changes

**Example of What Not To Do**:
```glsl
// BAD for tile-based GPU
vec4 color = texture(framebuffer_from_last_frame);  // ← Reading back kills performance
color += new_geometry_color;
output = color;
```

**Better Approach**:
```glsl
// GOOD for tile-based GPU
// Build geometry in screen space first
// Then composite in single pass
output = new_geometry_color;
```

---

## Memory Architecture

### PS2 Memory Layout

```
Total:           32 MB
- System RAM:    16 MB (game code, data, buffers)
- Embedded DRAM: 4 MB (GPU texture/framebuffer storage)
- Scratch Pad:   0 KB (used for I/O)
- LS (VU):       8 MB (Vector Unit local storage)

Game typical budget:
- Code:          2-3 MB
- Assets:        8-10 MB (textures, models)
- Runtime:       4-5 MB (physics, AI, state)
- Framebuffer:   2 MB
```

### Dreamcast Memory Layout

```
Total:           16 MB RAM
- System RAM:    8 MB (Dreamcast OS, game code)
- User RAM:      8 MB (game uses this)

VRAM (separate):  8 MB (textures, framebuffer)
- Texture Pool:   ~6-7 MB usable
- Framebuffer:    ~1 MB
- Vertex Buffers: Shared with texture pool

Game typical budget (estimate):
- Code:          2 MB
- Game Data:     2-3 MB
- Runtime:       2-3 MB (tight constraints)
- Audio Buffers: 1 MB
```

### The Memory Challenge

**PS2**: 16 MB for game + 4 MB for GPU = 20 MB available

**Dreamcast**: 8 MB for game + 8 MB VRAM shared = 16 MB total available

**You're losing ~4 MB of usable space.** This means:
- Asset streaming becomes necessary (load from GD-ROM as needed)
- Fewer objects in memory at once
- More aggressive garbage collection
- Possible scene simplifications

---

## GPU VRAM: The Advantage

### Why Dreamcast's Larger VRAM Helps

**PS2**: 4 MB VRAM for textures + framebuffer
- Extreme texture compression necessary (2-4 bit formats)
- Framebuffer: 2 MB at 512x224 resolution

**Dreamcast**: 8 MB VRAM for textures + framebuffer
- Can use less aggressive compression (PVRTC 4bpp instead of 2bpp)
- Better texture quality at same size
- More complex framebuffer operations possible

**Practical Benefit**: You can extract PC version textures (higher quality) and still fit them in VRAM.

**Calculation**:
```
PS2 texture budget:     4 MB VRAM - 2 MB framebuffer = 2 MB textures
Dreamcast budget:       8 MB VRAM - 1 MB framebuffer = 7 MB textures

With PVRTC compression (4bpp):
- 1 MB uncompressed texture → 0.125 MB PVRTC
- PS2 capacity:  ~16 textures worth
- Dreamcast:     ~56 textures worth (3.5x more texture memory!)
```

**This is your saving grace.** You can have much better looking textures than original PS2.

---

## CPU Performance: The Bottleneck

### What Gets CPU-Bound on Dreamcast

**Physics Calculations**: Simulating 50 rigid bodies = slow
**AI Pathfinding**: Complex A* pathfinding for multiple enemies
**Animation**: Blending multiple animations + IK solutions
**Audio Processing**: Real-time audio effects, mixing
**Collision Detection**: Many NPCs = many collision checks

### What Stays Fast

**Rendering**: GPU handles geometry (mostly)
**Asset Loading**: GD-ROM I/O is fast enough
**Input Processing**: Simple, per-frame

### Real Example: MGS2 AI

MGS2 has sophisticated guard AI:
- Vision system (line-of-sight traces)
- Pathfinding (dynamic paths around obstacles)
- Sound detection (reacts to noises)
- State machine (alert/combat/calm)

**On PS2**: 10+ guards simultaneously, full AI per frame
**On Dreamcast**: Likely need to:
- Reduce guard count (5-8 max)
- Simplify pathfinding (grid-based instead of full A*)
- Lower detection sensitivity (fewer traces)

---

## Rendering: Where the Real Work Happens

### MGS2 Original Rendering Pipeline

```
Input (3D geometry, textures, lighting)
    ↓
Vertex Shader (transform, animation)
    ↓
Rasterization
    ↓
Pixel Shader (lighting, normal maps, specular)
    ↓
Post-Processing (motion blur, film grain)
    ↓
Output (framebuffer)
```

### Your Dreamcast Port Rendering

```
Input (same, but LOD-reduced geometry)
    ↓
Vertex Shader (rewritten for GLSL/PowerVR)
    ↓
Rasterization (tile-based, 32x32 tiles)
    ↓
Pixel Shader (simplified—fewer effects)
    ↓
Post-Processing (optional, expensive)
    ↓
Output (framebuffer)
```

### Complexity Budget

| Feature | PS2 | Dreamcast | Notes |
|---------|-----|----------|-------|
| Normal Mapping | Yes | Yes | Essential for quality |
| Parallax Mapping | Yes | Simplified | High-cost, reduce |
| Specular Highlights | Yes | Yes | Keep for quality |
| Shadow Mapping | Yes | No | Use baked shadows instead |
| Motion Blur | Yes | No | Too expensive |
| Post-Processing | Yes | Limited | Keep essential only |
| Particle Effects | Yes | Reduced | Fewer particles, simpler effects |
| Reflection | Yes | No | Use precomputed cubemaps only |

---

## Audio: Mostly Compatible

### PS2 Audio (SCPH-10000)

```
Format:          48 kHz, 16-bit, stereo
Max Channels:    48 (through ISA Sound Processor)
Typical Usage:   8-16 channels game audio + voice
Effects:         Reverb, delay, pitch shifting
```

### Dreamcast Audio

```
Format:          48 kHz, 16-bit, stereo (Yamaha AICA)
Max Channels:    64 (technically)
Typical Usage:   8-16 channels (software mixing)
Effects:         Software-based (reverberation, filters)
```

**Bottom Line**: Audio system is compatible. Same sample rate, similar capabilities. Should port with minimal changes.

---

## Disc I/O

### PS2 GD-ROM

```
Speed:           4x speed (4.3 MB/s)
Capacity:        700 MB dual-layer
Latency:         ~80ms seek time
Format:          Proprietary PS2 filesystem
```

### Dreamcast GD-ROM

```
Speed:           12x speed (18 MB/s)
Capacity:        700 MB single-layer
Latency:         ~40ms seek time
Format:          ISO 9660 (standard)
Advantage:       Faster I/O than PS2!
```

**Good News**: Dreamcast GD-ROM is actually faster. If assets fit on disc, loading is quicker.

**Realistic Use**: Stream audio, load new scenes from disc as needed.

---

## Network: Bonus Feature

**PS2**: No built-in networking (Network Adapter sold separately)

**Dreamcast**: 10/100 Ethernet built-in

**Opportunity**: Could implement GD-ROM streaming over network for development (load assets from PC while debugging). Not needed for final port.

---

## Performance Target Calculation

### Given Constraints

```
PS2 MGS2:      60 FPS (target)
Dreamcast CPU: 33% slower
Dreamcast GPU: ~40% less fillrate

Realistic Target: 20-30 FPS
- At 20 FPS: Playable, acceptable for POC
- At 30 FPS: Very good, target for optimized version
- At 60 FPS: Not realistic without major cuts

Frame Budget (at 30 FPS):
- Per-frame time: 33.3 ms
- Rendering: ~20 ms (GPU work)
- Game Logic: ~10 ms (AI, physics, input)
- Overhead: ~3 ms (OS, asset streaming)
```

---

## Key Insights for Your Port

### What You Get

1. **Texture VRAM advantage** — Use PC version textures, still fit in VRAM
2. **Fast disc I/O** — Dreamcast faster than PS2, good for streaming
3. **Same audio system** — Audio port should be straightforward
4. **Standard ISA** — GCC compiler means standard C/C++ works

### What You Lose

1. **CPU speed** — 33% slower, no way around it
2. **Total memory** — 4 MB less usable RAM, requires optimization
3. **GPU architecture** — Tile-based rendering is different from immediate-mode
4. **Polygon budget** — 1/3 the geometry processing power

### Strategic Implications

- **Prioritize GPU optimization** over CPU (where performance matters)
- **Use VRAM advantage** for high-quality textures
- **Plan for reduced polygon count** from day one
- **Accept 20-30 FPS** as success metric
- **Consider gameplay simplifications** if CPU is bottleneck

---

## Cross-References

- For feasibility analysis based on this hardware: See `03_THEORETICAL_FEASIBILITY.md`
- For rendering backend implementation: See `05_IMPLEMENTATION_ROADMAP.md`
- For optimization strategies: See `06_QUICK_REFERENCE.md`
