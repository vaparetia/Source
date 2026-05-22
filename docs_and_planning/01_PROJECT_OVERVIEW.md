# 01. MGS2 Dreamcast Port — Project Overview

## Project Statement

**Goal**: Create a proof-of-concept playable port of Metal Gear Solid 2 to the Sega Dreamcast console.

**Scope**: Playable game state (gameplay functional, not feature-complete)

**Target Platform**: Sega Dreamcast (128 MB RAM, ~200 MHz SH-4 CPU, PowerVR GPU)

**Success Criteria**:
- ✓ Game boots and runs on Dreamcast
- ✓ Player can control character (basic input)
- ✓ 3D rendering works (geometry displays)
- ✓ Achieves 20-30 FPS (acceptable for POC)
- ✓ At least one playable scene/chapter

---

## Why This Project?

### Technical Learning Goals
1. **Cross-platform game porting** — understand architecture abstraction
2. **Reverse engineering** — extract and identify unknown file formats
3. **Hardware optimization** — work within tight Dreamcast constraints
4. **Rendering pipeline** — bridge DirectX to PowerVR/OpenGL ES
5. **Asset pipeline** — conversion and compression for retro hardware

### Historical Significance
- MGS2 was a PS2 flagship title — never officially released for Dreamcast
- Dreamcast was discontinued 2001; MGS2 released 2001 — missed opportunity
- This explores "what if Konami had ported it?"

### Why Dreamcast (and Not Another Platform)?
- **Challenge**: Significantly weaker hardware than original PS2 port
- **Relevance**: Open-source development community (KallistiOS)
- **Interest**: Demonstrates feasibility of modern game ports to retro hardware

---

## Core Challenge: Hardware Mismatch

### The Reality
```
                    PS2 (Original)      Dreamcast (Port Target)   Ratio
CPU Speed           300 MHz             200 MHz                   0.67x
CPU Architecture    MIPS + Vector       SH-4 + Intrinsics         Different ISA
VRAM                4 MB                8 MB                      2x (advantage)
RAM (total)         32 MB               16 MB                     0.5x
GPU Type            Graphics Synth.     PowerVR Tile-Based        Different
Polygon Budget      ~10M/frame          ~3-5M/frame               0.3-0.5x
Audio Channels      48kHz stereo        48kHz stereo              Same
```

**Key Insight**: Dreamcast has *more VRAM* but *less overall memory* and *slower CPU*.

This means:
- ✓ Textures can be higher quality (more VRAM)
- ✗ Game logic must run faster (slower CPU)
- ✗ Fewer polygons on screen (weaker GPU)

---

## What You're Actually Building

### Not A Direct Port
This isn't copy-paste code. You're:
1. **Extracting assets** from retail game (textures, models, audio)
2. **Recompiling game logic** with KallistiOS toolchain
3. **Writing a new rendering backend** (DirectX → PowerVR/OpenGL ES)
4. **Optimizing everything** to run on slower CPU

### What You're Keeping
- Game logic (AI, physics, animation, cutscenes)
- Audio system (recompiled for KallistiOS)
- Input handling (adapted to Dreamcast controller)
- Save game system (adapted to Dreamcast memory card)

### What You're Replacing
- **Rendering backend** (DirectX/Vita GPU → PowerVR/OpenGL ES)
- **Asset formats** (DDS textures → PVRTC, compiled models → Dreamcast format)
- **Shader system** (HLSL → GLSL)
- **File I/O** (PS2 disc / Vita FIOS → Dreamcast GD-ROM or network)

---

## Project Phases (High Level)

### Phase 0: Planning & Research (Week 1)
- ✓ Understand source code architecture
- ✓ Identify asset formats
- ✓ Plan extraction strategy
- ✓ Set up development environment

**You are here**

### Phase 1: Foundation (Weeks 2-3)
- Set up KallistiOS toolchain
- Create rendering backend stub
- Implement file support layer
- Get game logic compiling

### Phase 2: Rendering (Weeks 4-6)
- Implement GPU rendering calls
- Port/convert shaders
- Render first test geometry
- Achieve 20+ FPS with simple scene

### Phase 3: Integration (Weeks 7-8)
- Link game logic with rendering
- Implement input handling
- Audio system integration
- Load and render actual game assets

### Phase 4: Optimization & Polish (Weeks 9-10)
- Profile CPU bottlenecks
- Optimize hotspots
- Adjust LOD/graphics settings
- Testing and bug fixes

### Phase 5: Completion (Week 11+)
- Full game bootable
- At least one scene playable
- Documentation complete

**Total Estimate**: 3-4 months solo (or 6-8 weeks with collaborators)

---

## Critical Dependencies

### Required
1. **MGS2 Source Code** ✓ (you have this)
2. **Retail MGS2 Copy** (PC version recommended for asset extraction)
3. **KallistiOS SDK** (free, open-source Dreamcast development)
4. **GCC Cross-Compiler** (sh-elf-gcc)
5. **Asset Extraction Tools** (QuickBMS, Noesis, ImageMagick, etc.)

### Nice to Have
- Dreamcast hardware or emulator (Demul, Flycast)
- 3D graphics knowledge (OpenGL/DirectX)
- Experience with reverse engineering
- Japanese language skills (for code comments)

### Not Required But Helpful
- Previous Dreamcast development experience
- Shader programming knowledge
- Game engine optimization experience

---

## Major Unknowns (Discovery Phase)

### Format Questions
1. **Model Format (`.mdl`)**: Is it Quake-based? Proprietary? Unknown
   - **Impact**: Determines asset extraction difficulty
   - **Mitigation**: Noesis or community tools might auto-detect

2. **Archive Format (`.psarc`)**: Is source available? Do we need QuickBMS?
   - **Impact**: Determines ease of asset extraction
   - **Mitigation**: PC version might use loose files (easier)

3. **Shader Format**: Are source shaders in `.cg` or pre-compiled?
   - **Impact**: Determines shader porting complexity
   - **Mitigation**: Can rewrite shaders from scratch if needed

### Technical Questions
4. **Memory Layout**: How much runtime memory does game use?
   - **Impact**: Can it fit in Dreamcast's 16 MB?
   - **Mitigation**: Profile on PC version first

5. **CPU Bottleneck**: Which systems are CPU-bound?
   - **Impact**: Determines what optimizations are needed
   - **Mitigation**: Profiling and selective optimization

6. **Asset Quality**: Will extracted assets look acceptable on Dreamcast?
   - **Impact**: May need manual optimization
   - **Mitigation**: Progressive quality increases (start low, improve)

---

## Success Metrics (How You'll Know It's Working)

### Milestone 1: Builds & Boots (Week 3)
- [ ] Code compiles for Dreamcast without errors
- [ ] Game boots (shows splash screen or first scene)

### Milestone 2: Renders Geometry (Week 6)
- [ ] 3D objects visible on screen
- [ ] Textures display (even if simple)
- [ ] Achieves >15 FPS

### Milestone 3: Game Logic Works (Week 8)
- [ ] Player can move character
- [ ] Camera responds to input
- [ ] Animation plays

### Milestone 4: Playable (Week 10+)
- [ ] Can complete simple gameplay sequence
- [ ] Save/load works
- [ ] Consistent 20+ FPS

---

## Known Risks

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|-----------|
| Asset formats undocumented | Medium | High | Use community tools (Noesis, QuickBMS) |
| CPU too slow for game logic | Medium | High | Selective optimization, scene simplification |
| GPU architecture too different | Low | High | Pre-compile pipelines, plan shaders carefully |
| Memory insufficient | Medium | Medium | Profile early, aggressive memory management |
| Shader conversion complex | Medium | Medium | Start simple, iterative improvement |
| Development tools unreliable | Low | Medium | Use established tools (KallistiOS, GCC) |

---

## What This *Isn't*

- Not a commercial product (proof-of-concept only)
- Not a full feature-complete game (playable POC is success)
- Not a replacement for original game (compatibility hack, not remake)
- Not meant for distribution (asset extraction illegal for redistribution)
- Not a tutorial for beginners (requires solid technical knowledge)

---

## Next Steps

1. **Read Hardware Comparison** (`02_HARDWARE_COMPARISON.md`)
   - Understand what you're building for

2. **Review Feasibility Analysis** (`03_THEORETICAL_FEASIBILITY.md`)
   - Confirm this is actually doable

3. **Study Asset Extraction Guide** (`04_ASSET_EXTRACTION_GUIDE.md`)
   - Plan how to get assets from the game

4. **Create Implementation Plan** (`05_IMPLEMENTATION_ROADMAP.md`)
   - Detail your specific approach

5. **Set Up Development Environment**
   - Install KallistiOS, toolchain, extraction tools

---

## Document Cross-References

- For hardware specs: See `02_HARDWARE_COMPARISON.md`
- For technical feasibility: See `03_THEORETICAL_FEASIBILITY.md`
- For asset extraction: See `04_ASSET_EXTRACTION_GUIDE.md`
- For step-by-step implementation: See `05_IMPLEMENTATION_ROADMAP.md`
- For command reference: See `06_QUICK_REFERENCE.md`
