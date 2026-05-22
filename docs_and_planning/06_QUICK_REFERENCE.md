# 06. Quick Reference — Commands, Tools & Resources

Quick lookup guide for commands, tools, and resources you'll need during development.

## Development Environment Commands

### KallistiOS Setup (One-Time)

```bash
# Clone KallistiOS
git clone https://github.com/KallistiOS/KallistiOS.git

# Set environment variables
export KOS_BASE=/path/to/KallistiOS
export PATH=$KOS_BASE/utils/bin:$PATH

# Build a test program
cd $KOS_BASE/examples/dreamcast/hello
make
./hello.elf
```

### Compilation Commands

```bash
# Compile single file for Dreamcast
sh-elf-g++ -c -I/path/to/KOS_BASE/include source.cpp

# Compile and link
sh-elf-gcc -O2 -fno-exceptions *.c *.cpp -o game.elf -lkos

# Check compilation errors
make 2>&1 | grep error | head -20
```

### Testing & Emulation

```bash
# Run on Flycast emulator
flycast game.elf

# Run on Demul emulator
demul -rom game.elf

# Debug output (many emulators support serial logging)
cat debug_log.txt
```

---

## Asset Extraction Commands

### Using QuickBMS

```bash
# Extract from PSARC archive
quickbms.exe script.bms data.psarc output_folder/

# List files in archive without extracting
quickbms.exe -l script.bms data.psarc

# Verbose output to diagnose problems
quickbms.exe -v script.bms data.psarc output_folder/
```

### Using 7-Zip

```bash
# Extract ZIP archives
7z x archive.zip -ooutput_folder/

# List contents
7z l archive.zip | head -50

# Extract specific file
7z x archive.zip -ooutput/ specific_file.txt
```

### Using ImageMagick

```bash
# Convert DDS to PNG
convert input.dds output.png

# Batch convert all DDS in folder
for f in *.dds; do convert "$f" "${f%.dds}.png"; done

# Resize texture (example: half size)
convert input.dds -resize 50% output.dds

# Check image properties
identify input.dds
```

### Using FFmpeg

```bash
# Convert WAV to OGG (Vorbis compression)
ffmpeg -i input.wav -c:a libvorbis -q:a 5 output.ogg

# Extract audio from video
ffmpeg -i video.mp4 -q:a 0 -map a audio.wav

# Batch convert audio
for f in *.wav; do ffmpeg -i "$f" "${f%.wav}.ogg"; done

# Check audio properties
ffprobe input.wav
```

---

## File Inspection Commands

### Hexdump (View Binary Files)

```bash
# Show first 20 lines of hex
hexdump -C input_file | head -20

# Show specific byte range
hexdump -C input_file | grep "^000000"

# Show ASCII alongside hex
hexdump -C input_file | less

# Search for magic number
hexdump -C input_file | grep "5045 4143"  # "PEAC" example
```

### File Type Identification

```bash
# Identify file type
file input_file

# Show detailed MIME type
file -i input_file

# For multiple files
file *.* | grep -i "data\|archive"

# Check if file is text or binary
file -b input_file | grep -q "text" && echo "Text" || echo "Binary"
```

### String Extraction

```bash
# Extract human-readable strings from binary
strings input_file | head -50

# Search for specific patterns
strings input_file | grep -i "texture\|model\|format"

# Find file paths in binary
strings input_file | grep "\./"
```

---

## Directory Organization Commands

### Project Structure Setup

```bash
# Create standard project layout
mkdir -p {src,include,assets,build,docs,tools,scripts}
mkdir -p src/{graphics,audio,input,game_logic,platform}
mkdir -p assets/{textures,models,audio,data}

# Initialize git
git init
git config user.name "Your Name"
git config user.email "your@email.com"

# Create .gitignore
cat > .gitignore << 'GITIGNORE'
*.o
*.elf
*.a
*.dol
build/
assets/extracted/
temp/
.vscode/
GITIGNORE
```

### Batch Operations

```bash
# List all C++ files in project
find . -name "*.cpp" -o -name "*.h" | sort

# Count lines of code
find . -name "*.cpp" -o -name "*.h" | xargs wc -l

# Copy entire directory structure
cp -r source_dir dest_dir

# Remove all object files
find . -name "*.o" -delete

# List only extracted files (not originals)
ls -lh assets/extracted/ | sort -k5 -h
```

---

## Git Commands for Version Control

```bash
# Check status
git status

# Stage changes
git add .

# Commit changes
git commit -m "Phase 2: Implement rendering backend"

# View history
git log --oneline | head -20

# Create branch for experimental feature
git branch feature/shader-optimization
git checkout feature/shader-optimization

# Merge branch back to main
git checkout main
git merge feature/shader-optimization

# View what changed
git diff HEAD~1 HEAD
```

---

## Debugging Commands

### GDB Debugger (Dreamcast)

```bash
# Start debugger with program
sh-elf-gdb game.elf

# Set breakpoint
(gdb) break main
(gdb) break filename.cpp:42

# Run program
(gdb) run

# Step through code
(gdb) step           # Step into function
(gdb) next           # Step over function
(gdb) continue       # Continue to next breakpoint

# Inspect variables
(gdb) print variable_name
(gdb) print *pointer_variable
(gdb) info locals

# Backtrace (see call stack)
(gdb) backtrace
(gdb) frame 0
```

### Printf Debugging

```cpp
// Simple debug output for Dreamcast
printf("Value: %d\n", variable);
printf("Pointer: %p\n", ptr);
printf("String: %s\n", str);
fflush(stdout);  // Flush buffer immediately
```

---

## Performance Profiling Commands

### Measure Code Execution Time

```cpp
#include <time.h>

clock_t start = clock();
// Code to profile
clock_t end = clock();
double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
printf("Elapsed: %.3f seconds\n", elapsed);
```

### Memory Profiling

```cpp
// Track memory allocation
#include <malloc.h>

size_t mem_before = malloc_stats();
// Code to profile
size_t mem_after = malloc_stats();
printf("Memory used: %zu bytes\n", mem_after - mem_before);
```

---

## Essential Tools & Downloads

| Tool | Purpose | URL |
|------|---------|-----|
| KallistiOS | Dreamcast SDK | https://github.com/KallistiOS/KallistiOS |
| Noesis | Model viewer/converter | https://richwhitehouse.com/index.php?content=inc_projects.php&showproject=91 |
| QuickBMS | Archive extractor | https://aluigi.altervista.org/quickbms.html |
| ImageMagick | Image conversion | https://imagemagick.org/ |
| FFmpeg | Audio/video conversion | https://ffmpeg.org/ |
| GCC (sh-elf) | Cross-compiler | Part of KallistiOS |
| Flycast | Dreamcast emulator | https://github.com/flyinghead/flycast |
| Demul | Dreamcast emulator | http://demul.emulation64.com/ |
| HxD | Hex editor (Windows) | https://mh-nexus.de/en/hxd/ |
| VSCode | Code editor | https://code.visualstudio.com/ |

---

## Documentation URLs

| Resource | URL |
|----------|-----|
| KallistiOS Docs | https://dreamcast-talk.com/forum/ |
| Dreamcast Dev Wiki | https://dreamcast.wiki/ |
| Game Format Database | https://xentax.com/ |
| QuickBMS Scripts | https://aluigi.altervista.org/bms/ |
| Zenhax Forums | https://zenhax.com/ |
| Reverse Engineering 101 | https://www.youtube.com/playlist?list=PLhixgUqwRTjxglIswKp9mpkfPNfHkzyeY |

---

## File Format References

### DDS (DirectX Texture Format)

```
Header Info:
Magic: "DDS " (0x20534444)
Size: 124 bytes
Flags: DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT
Format: Compression type (DXT1, DXT5, etc.)
```

### WAV (Audio Format)

```
Chunk 1: RIFF header
Chunk 2: fmt (format specification)
Chunk 3: data (actual audio)

Properties: Sample rate, bit depth, channels
```

### PSARC (PlayStation Archive)

```
Header: Proprietary format
Version: Varies by console
Compression: Usually zlib
Structure: Unknown without reverse engineering
```

---

## Common Errors & Fixes

| Error | Likely Cause | Fix |
|-------|--------------|-----|
| `undefined reference to '__main'` | Missing entry point | Ensure main() or KOS entry point defined |
| `file not found: crt0.o` | KallistiOS not in PATH | Check KOS_BASE environment variable |
| `Memory allocation failed` | Out of Dreamcast RAM | Profile memory, implement streaming |
| `framebuffer read error` | GPU immediate-mode assumption | Use tile-based rendering approach |
| `Missing symbol: _ZN...` | Undefined function | Check linker flags, link against libraries |
| `Floating point exception` | Division by zero or FPU issue | Add null checks, handle edge cases |

---

## Pro Tips

### 1. Keep a Development Log
```bash
cat >> DEVLOG.md << 'LOG'
## [Date] - [Time Spent] hrs
- What you accomplished
- Blockers encountered
- Next steps

Examples:
## May 22 - 2 hrs
- Extracted 500 texture files
- Noesis recognized .mdl as Quake format ✓
- Next: Batch convert DDS to PVRTC

## May 23 - 3 hrs
- Implemented file I/O wrapper
- Got game logic compiling
- Blocker: Shader conversion still unknown
LOG
```

### 2. Create Checklists
```markdown
# MGS2 Dreamcast Phase 2 Rendering
- [ ] GPU context initialization
- [ ] Vertex buffer creation
- [ ] Triangle rendering test
- [ ] Texture loading
- [ ] Shader compilation
- [ ] Performance baseline (must be >15 FPS)
```

### 3. Automate Repetitive Tasks
```bash
# Create script for batch texture conversion
#!/bin/bash
for dds in *.dds; do
    convert "$dds" "${dds%.dds}.png"
    echo "Converted $dds"
done
```

### 4. Document Discoveries
Whenever you learn something important, add it to your research notes:
```markdown
# File Format Discovery
## Date: May 22

### .mdl Format
- Recognized by Noesis as Quake MDL variant
- 4-byte magic number: 0x4D 0x44 0x4C 0x20 ("MDL ")
- Can export to FBX using Noesis script
- Quality: Acceptable for Dreamcast (polygons minimal)

### Next: Test FBX import pipeline
```

---

## Useful One-Liners

```bash
# Count total source lines
find . -name "*.cpp" -o -name "*.h" | xargs wc -l | tail -1

# Find most common file extensions
find . -type f | sed 's/.*\.//' | sort | uniq -c | sort -rn

# List largest files
find . -type f -exec du -h {} + | sort -rh | head -20

# Search for hardcoded paths (may reveal format info)
grep -r "\.mdl\|\.dds\|\.psarc" . --include="*.cpp" --include="*.h"

# Find TODO comments in code
grep -r "TODO\|FIXME\|HACK" . --include="*.cpp" --include="*.h"

# Compare two files side-by-side
diff -u file1.cpp file2.cpp | less
```

---

## When You Get Stuck

1. **Search existing solutions first**
   - GitHub: `mgs2 dreamcast`
   - Forums: Zenhax, KallistiOS Discord
   - Reddit: r/retrogaming, r/reversengineering

2. **Ask specific questions**
   - Show what you've tried
   - Include error messages
   - Provide context (what you're porting, which format)

3. **Take a break**
   - Step away for 30 minutes
   - Many solutions come after rest
   - Avoid tunnel vision

4. **Use your contingency plans**
   - Stuck on model format? Try Noesis or reach out
   - CPU too slow? Use simpler AI
   - Shader too complex? Use fixed function pipeline

---

## Success Metrics Checklist

Use this to verify progress at key milestones:

### Week 2 Completion
- [ ] Codebase fully understood
- [ ] Asset extraction strategy confirmed
- [ ] KallistiOS working on test program
- [ ] Asset tools installed and tested

### Week 4 Completion
- [ ] Game logic compiles without major errors
- [ ] All platform stubs in place
- [ ] File I/O wrapper complete

### Week 6 Completion
- [ ] Colored triangle renders on Dreamcast
- [ ] Can load and display texture
- [ ] Basic shaders compiled and working

### Week 10 Completion
- [ ] Game models displaying
- [ ] Player character visible
- [ ] Achieves 20+ FPS

### Week 12 Completion
- [ ] First chapter playable
- [ ] Can move character, interact
- [ ] No major bugs blocking gameplay
