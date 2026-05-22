# 05. Implementation Roadmap — Phase-by-Phase Plan

## Overview

This document outlines the specific steps you'll take to build the Dreamcast version of MGS2.

**Total Timeline**: 3-4 months
**Effort**: Full-time solo development
**Complexity**: Medium-High

---

## Phase 0: Research & Planning (Weeks 1-2)

### Objectives
- [ ] Understand codebase structure completely
- [ ] Discover unknown file formats
- [ ] Set up development environment
- [ ] Plan extraction strategy
- [ ] Create project tasks/schedule

### Week 1: Codebase Mastery

**Day 1-2**: Repository Walkthrough
```bash
cd mgs2-source

# Map the directory structure
find . -type d -maxdepth 2 | sort

# Understand file counts by type
find . -name "*.cpp" | wc -l      # Should be ~5000
find . -name "*.h" | wc -l
find bp/shared -type f | wc -l    # Game logic files

# Identify key systems
grep -r "class.*Renderer" bp/Source/Renderer/ | head -20
grep -r "class.*FileSupport" bp/shared/ | head -10
```

**Day 3-4**: Platform-Specific Code Identification
```bash
# Find all platform backends
find bp/Source/Renderer/Base/Backend -type d

# Examine one rendering backend (X360) as reference
ls -la bp/Source/Renderer/Base/Backend/X360/

# Study the abstract interface
cat bp/Source/Renderer/Base/Backend/CRenderBackend.h | head -100
```

**Day 5**: Documentation Review
- Read comments in platform-specific files (especially Japanese ones via translator)
- Look for internal documentation or design notes
- Identify what was kept vs. removed between platform ports

**Deliverable**: `research_notes/codebase_structure.md`

### Week 2: Format Discovery & Environment Setup

**Day 1-2**: File Format Investigation

**Action Item 1**: Search for `.mdl` references
```bash
grep -r "\.mdl" bp/shared/ --include="*.cpp" --include="*.h"
grep -r "LoadModel\|model.*load" bp/shared/ -i
grep -r "Quake\|mdl.*format" bp/shared/
```

**Action Item 2**: Extract a game copy (if you have one)
- Obtain retail PC version of MGS2
- Locate asset files on disc or in installation
- Identify directory structure

**Action Item 3**: Test format detection tools
- Download and install Noesis
- Extract one `.mdl` file (any model from the game)
- Test if Noesis recognizes format
- Document result

**Action Item 4**: Research community
- Search GitHub: "MGS2 extraction"
- Check Zenhax forums for MGS2 format info
- Search GitHub for "mgs2" + "tool"
- Document any existing solutions

**Day 3-4**: Development Environment Setup

**KallistiOS Installation**:
```bash
# On your development machine
# Follow official KallistiOS guide:
# https://dreamcast-talk.com/forum/viewtopic.php?t=575

# Basic steps:
# 1. Install prerequisites (GCC, etc.)
# 2. Clone KallistiOS repo
# 3. Build toolchain
# 4. Verify installation (build hello world)
```

**Test KallistiOS**:
```bash
# Create simple test program
cd /path/to/kos/examples/dreamcast/hello

# Build
make

# Test on emulator (Demul or Flycast)
./hello.elf
```

**Day 5**: Asset Tools Installation
```bash
# Install Noesis (free, download from website)
# Install QuickBMS (free, from https://aluigi.altervista.org/)
# Install ImageMagick (ffmpeg alternative)
# Install 7-Zip and/or WinRAR

# Test each tool:
# - Noesis: Try opening various 3D model files
# - QuickBMS: Run a sample script
# - ImageMagick: Convert a test image
```

**Deliverable**: 
- `research_notes/file_formats.md` (findings about model format)
- Confirmed working KallistiOS setup
- List of asset extraction tools installed

### Week 2 Summary Deliverables

- [ ] Complete codebase map created
- [ ] Model format identified or approach determined
- [ ] Asset extraction strategy documented
- [ ] KallistiOS environment working
- [ ] Asset tools tested and ready

---

## Phase 1: Foundation (Weeks 3-4)

### Objectives
- [ ] Extract core game files from retail version
- [ ] Create Dreamcast project structure
- [ ] Get game logic compiling for SH-4
- [ ] Implement KallistiOS support layer

### Week 3: Project Setup & Compilation

**Day 1-2**: Create KallistiOS Project Structure
```bash
mkdir -p mgs2-dreamcast/{
  src,
  include,
  assets,
  build,
  tools,
  docs
}

# Copy game logic from original source
cp -r /path/to/mgs2-source/bp/shared mgs2-dreamcast/src/game_logic

# Create basic KallistiOS project structure
# (following KallistiOS examples format)
```

**Day 3-4**: Platform Abstraction Layer

Create wrappers for KallistiOS:
```cpp
// src/kos/BP_FileSupport_KOS.cpp
#include "Engine/Stdafx.h"
#include "BP_FileSupport.h"

void BP_InitializeFileSupport() {
    // Initialize KallistiOS filesystem
}

void * BP_OpenFile(const char * path, const char * commonPath) {
    // Use KallistiOS file I/O
    FILE * fp = fopen(path, "rb");
    return fp;
}
// ... more file I/O stubs
```

**Day 5**: Test Compilation

```bash
# Create minimal test build
# Include just game logic, no rendering

# Try to compile:
# sh-elf-g++ -c src/game_logic/BP_Memory.cpp

# Expected: Should compile without major errors
# Likely: Will find missing includes to fix
```

**Deliverable**: Basic project compiling, even if incomplete

### Week 4: KallistiOS Integration

**Day 1-3**: Create Support Layers

```cpp
// src/kos/BP_AudioSupport_KOS.cpp
// src/kos/BP_InputSupport_KOS.cpp
// src/kos/BP_MemorySupport_KOS.cpp
```

**Day 4-5**: Get Full Logic Compiling

```bash
# Build entire game logic for Dreamcast
# Resolve all compilation errors
# Create list of required platform implementations

# Expected output: Complete symbol list of what needs implementation
```

**Deliverable**: Game logic compiles completely, ready for rendering backend

---

## Phase 2: Rendering (Weeks 5-6)

### Objectives
- [ ] Implement DreamcastCRenderBackend
- [ ] Get GPU context working
- [ ] Render first test geometry
- [ ] Convert/optimize shaders

### Week 5: GPU Framework

**Day 1-2**: Implement GPU Initialization

```cpp
// src/graphics/DreamcastCRenderBackend.h
class DreamcastCRenderBackend : public CRenderBackend {
public:
    virtual void Initialize(...) override;
    virtual void Shutdown() override;
    virtual void BeginFrame() override;
    virtual void EndFrame() override;
    virtual void Present() override;
    // ... more methods
};
```

**Day 3-4**: Test Basic Rendering

```cpp
// Can you render a single colored triangle?
// - Set up vertex buffer
// - Issue draw call
// - Present to screen
// - See colored triangle on Dreamcast
```

**Day 5**: Texture Support

```cpp
// Implement texture loading/binding
// Test: Load a `.dds` file from PC
// Render textured quad
```

**Deliverable**: Single colored triangle rendering on Dreamcast

### Week 6: Shader Implementation

**Day 1-2**: Shader Conversion

```glsl
// Start with simplest shader: basic lighting
// vertex shader: just transform position
// pixel shader: flat color

// Get working, then expand
```

**Day 3-4**: Advanced Shaders

```glsl
// Normal map lighting
// Specular highlights
// UV animation
// Multiple textures
```

**Day 5**: Optimization

- Batch similar objects
- Minimize state changes
- Test framerate

**Deliverable**: Rendering pipeline complete, can render complex geometry

---

## Phase 3: Asset Integration (Weeks 7-8)

### Objectives
- [ ] Extract all game assets
- [ ] Convert to Dreamcast formats
- [ ] Load and display actual game models/textures

### Week 7: Asset Extraction

**Day 1-5**: Full Extraction Pipeline

```bash
# Extract all textures
# Extract all models
# Extract all audio
# Extract all other assets

# Create cataloging system
# Organize assets by type

# Prepare for format conversion
```

**Deliverable**: All game assets extracted and organized

### Week 8: Format Conversion

**Day 1-2**: Texture Conversion

```bash
# DDS → TGA → PVRTC (PowerVR format)
# Batch process all textures
# Verify quality acceptable
```

**Day 3-4**: Model Conversion

```bash
# Determine exact `.mdl` format
# Extract to intermediate format (OBJ/FBX)
# Convert to Dreamcast-compatible format
# Load and render first game model
```

**Day 5**: Package Assets

```bash
# Create asset package format
# Load assets into memory efficiently
# Test streaming from GD-ROM
```

**Deliverable**: Game models and textures loading and rendering

---

## Phase 4: Integration & Optimization (Weeks 9-10)

### Objectives
- [ ] Link game logic with rendering
- [ ] Implement gameplay systems
- [ ] Profile and optimize

### Week 9: Full Integration

**Day 1-3**: Marry Logic & Rendering

```cpp
// Game logic calls:
// - CRenderBackend::DrawMesh()
// - CRenderBackend::SetTexture()
// - etc.

// Make sure everything calls correctly
// Test with simple gameplay scene
```

**Day 4-5**: Gameplay Features

- Input handling
- Animation playback
- Audio integration

**Deliverable**: Game logic and rendering working together

### Week 10: Profiling & Optimization

**Day 1-2**: Profile CPU Usage

```bash
# Identify hotspots
# What's using 90% of CPU time?
# - Physics?
# - AI?
# - Animation?
```

**Day 3-4**: Target Optimization

```cpp
// Focus on top 3 CPU consumers
// Implement specific optimizations:
// - Cache commonly used values
// - Reduce NPC count if AI slow
// - Simplify physics if needed
```

**Day 5**: Framerate Target

- Achieve 20+ FPS consistently
- Document optimization changes
- Note what had to be simplified

**Deliverable**: Playable game at acceptable framerate

---

## Phase 5: Polish & Testing (Weeks 11+)

### Objectives
- [ ] Bug fixes
- [ ] Performance passes
- [ ] Complete at least one full scene

### Week 11: Gameplay Testing

- [ ] Complete first chapter playable
- [ ] All major systems functional
- [ ] Fix critical bugs

### Week 12: Final Polish

- [ ] Performance optimization
- [ ] Quality assurance
- [ ] Documentation

**Deliverable**: Complete, playable POC

---

## Key Milestones & Checkpoints

| Week | Milestone | Status Check |
|------|-----------|--------------|
| 2 | Codebase understood, formats discovered | Can explain architecture fully |
| 4 | Game logic compiling | `sh-elf-g++ mgs2-source` succeeds |
| 6 | First triangle rendering | See colored geometry on screen |
| 8 | Game assets displaying | See MGS2 models/textures |
| 10 | Game playable | Can move character, interact |
| 12 | POC complete | Can play 5+ minutes uninterrupted |

---

## Daily Work Rhythm (Suggested)

```
9:00 AM  - Review previous work, understand where you left off
9:15 AM  - Core coding work (2 hours, uninterrupted)
11:15 AM - Break + testing
11:30 AM - Continue coding (2 hours)
1:30 PM  - Lunch break
2:30 PM  - Integration/debugging (2 hours)
4:30 PM  - Documentation + planning tomorrow (1 hour)
5:30 PM  - End

Total: 7 productive hours/day
```

---

## Success Criteria per Phase

### Phase 0 Complete When:
- [ ] Can explain codebase structure without looking at code
- [ ] Know how to extract game assets
- [ ] Development environment is proven working

### Phase 1 Complete When:
- [ ] Game logic compiles completely
- [ ] Builds link without symbol errors
- [ ] Basic file I/O works

### Phase 2 Complete When:
- [ ] Can render a complex 3D scene
- [ ] Achieves 20+ FPS on simple geometry
- [ ] Shaders compile and work

### Phase 3 Complete When:
- [ ] All game assets loaded
- [ ] Can render actual game models
- [ ] Asset streaming works

### Phase 4 Complete When:
- [ ] Game logic and rendering fully integrated
- [ ] Achieves 20-30 FPS with real assets
- [ ] Can play at least 5 minutes uninterrupted

### Phase 5 Complete When:
- [ ] Full chapter playable
- [ ] All major gameplay systems working
- [ ] Consistent performance

---

## Contingency Plans

### If Model Format is Unknown (Worst Case)

**Backup Plan 1**: Use Noesis
- If Noesis can extract: Use existing tool
- Time impact: +2 hours

**Backup Plan 2**: Community Help
- Post on Zenhax forums
- Community may have solved it already
- Time impact: +24 hours (waiting for response)

**Backup Plan 3**: Reverse Engineer
- Manually analyze hex dumps
- Write custom extraction tool
- Time impact: +16 hours

**Backup Plan 4**: Placeholder Models
- Use simple geometric stand-ins
- Still playable, lower visual quality
- Time impact: +8 hours

### If CPU is Too Slow

**Backup Plan 1**: Reduce NPC Count
- Instead of 10 guards, use 5
- Still feels like MGS2
- Time impact: +4 hours

**Backup Plan 2**: Simplify AI
- Use pathfinding grid instead of full A*
- Reduced complexity but playable
- Time impact: +8 hours

**Backup Plan 3**: Lower Framerate Target
- Accept 15-20 FPS instead of 20-30
- Still playable, less smooth
- Time impact: 0 (just accept)

### If Rendering Too Complex

**Backup Plan 1**: Use Fixed Function Pipeline
- No shaders, use basic lighting
- Slower visually but simpler code
- Time impact: +8 hours

**Backup Plan 2**: Reduce Shader Complexity
- Fewer texture layers
- Simpler lighting model
- Time impact: +4 hours

---

## Tools & Resources to Keep Handy

```bash
# Version control (track changes)
git init mgs2-dreamcast
git config user.name "Your Name"

# Documentation templates
# Keep notes on every discovery

# Tool scripts (batch processing)
# Texture conversion
# Model extraction
# Asset cataloging

# Emulator (Flycast or Demul)
# For testing without hardware
```

---

## Next Steps

1. **Start Phase 0 immediately**
   - Begin Week 1 codebase walkthrough
   - Document findings

2. **Commit to schedule**
   - 3-4 months, 7 productive hours/day
   - Some variation expected

3. **Stay flexible**
   - Adjust plan based on discoveries
   - Don't get stuck on unknowns (use contingencies)
   - Ask for help when needed

---

## Success Definition

You'll know this project is successful when:

✓ Game boots on Dreamcast
✓ Player visible and controllable
✓ 3D rendering working
✓ Can play uninterrupted for 5+ minutes
✓ Achieves 20+ FPS

Anything beyond that is bonus.
