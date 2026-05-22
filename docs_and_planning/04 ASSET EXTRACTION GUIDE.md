# Asset Loading & Extraction Guide for MGS2 Dreamcast Port

## Part 1: Understanding the Asset Formats

### What the Source Code Tells Us

From analyzing `BP_FileSupportWin32.cpp`, `BP_FileSupportVTA.cpp`, and `BP_FileSupportPS3.cpp`:

**Archive Format: PSARC** (PlayStation Archive)

- Used by PS3 and Vita versions
- Contains ALL game assets (models, textures, audio)
- Files in source code reference: `*.psarc`
- Examples: `common.psarc`, `mgs2.psarc`, etc.

**Individual Asset Formats** (from grep results):

- **Audio**: `.wav` (uncompressed in archives, streamed)
- **Textures**: `.dds` (compressed DirectX texture format for PC)
- **Models**: `.mdl` (custom binary format, unknown)
- **Archives**: `.spa` (referenced in build files)

### Key Finding: PC Version Advantage

The Win32 version (`BP_FileSupportWin32.cpp`) loads assets **directly from individual files**, not archives:

- No PSARC decompression needed
- Higher quality source textures (often uncompressed or less compressed)
- More straightforward file structure

**Implication**: Extract from **PC version** rather than PS2/PS3.

-----

## Part 2: Asset Extraction From Retail Disc

### Option A: Extract from PS2 Disc

**Tools Required:**

1. **IsoBuster** (Windows, ~$25) or **7-Zip** (free)
- Can mount/browse PS2 ISO files
- Extract files from disc
1. **UMDGen** or **UMDTools** (if you have Vita version)
- PSVita format tools
- Can extract from VPK files
1. **QuickBMS** (free, powerful)
- Reverse engineering tool for unknown formats
- Can decompress archived files
- Supports custom scripts

**Process:**

```bash
# 1. Mount PS2 disc image (using IsoBuster or similar)
# 2. Browse file structure - typically:
#    /SLUS_xxxxx/  (main game folder)
#    /DATA/        (assets folder)

# 3. Look for archive files (.BIN, .DAT, etc.)
# 4. Export to folder
# 5. Attempt to decompress/identify formats
```

**Problem**: PS2 assets are heavily optimized for PS2 hardware

- Textures use proprietary compression
- Models have PS2-specific vertex formats
- You’ll need to reverse-engineer decompression

-----

### Option B: Extract from PC Version (RECOMMENDED)

**Tools Required:**

1. **7-Zip** or **WinRAR** (if assets are archived)
1. **QuickBMS** with proper MGS2 script (if custom archive)
1. **DirectX Texture Tool** (Microsoft, free)
- Convert `.dds` files to standard formats

**Process:**

**Step 1: Locate Asset Files**

```
Typical PC game install locations:
- C:\Program Files\Konami\MGS2\
- C:\Program Files (x86)\Konami\Metal Gear Solid 2\
- Game installation folder

Look for subdirectories:
- /data/
- /textures/
- /models/
- /sound/
- /movies/
```

**Step 2: Identify Archive Format**

```bash
# Use hexdump or file inspection tool
# Look for magic numbers/headers:

file data.bin      # Shows file type
hexdump -C data.bin | head -20  # Shows first bytes

# Common archive headers:
# 50 4B 03 04 = ZIP
# 7A 37 AF BC = 7z
# PSARC (custom Konami format)
```

**Step 3: Extract Archives**

```bash
# If ZIP format:
unzip -r data.zip -d extracted_assets/

# If custom PSARC, need QuickBMS script
# (see Part 3 below)

# If loose files, just copy:
cp -r textures/ extracted_assets/
cp -r models/ extracted_assets/
```

-----

## Part 3: Decompressing Unknown Formats

### Finding File Format Scripts with QuickBMS

QuickBMS is your best tool for unknown formats. It’s a scripting language for reverse-engineering.

**Installation:**

```bash
# Download from: https://aluigi.altervista.org/quickbms.html
# Extract and use from command line
```

**Finding Existing MGS2 Scripts:**

```bash
# Check these communities for existing scripts:
1. QuickBMS Forums: https://aluigi.altervista.org/bms/
2. XentaxBackup: Game format database
3. GitHub: Search "MGS2" + "QuickBMS" or "extraction"

# Popular script repositories:
- https://github.com/topics/quickbms
- https://xentax.com/index.php
```

**Creating Your Own Script (If Needed):**

QuickBMS scripts are text files. Basic structure:

```
# Example: Extract PSARC archive
open FDSE "input.psarc" 1
idstring "PSARC"           # Check magic number
goto 0x10
get num_files long
get archive_size long

for i = 0 < num_files
    get file_offset long
    get file_size long
    get filename string
    
    savepos pos
    goto file_offset
    
    # Save extracted file
    log filename 0 file_size
    
    goto pos
next i
```

**Running QuickBMS:**

```bash
quickbms.exe script.bms input_file.psarc output_folder/
```

-----

## Part 4: Texture Conversion Pipeline

Once you extract `.dds` files from PC version:

### Converting DDS to Portable Format

**Tool: Microsoft DirectX Texture Tool** (free, official)

```bash
# Installation: Part of DirectX SDK
# Command line:
texconv -f R8G8B8A8_UNORM input.dds -o output.tga

# Or use ImageMagick (cross-platform):
convert input.dds output.png
```

**For Dreamcast PVRTC Conversion:**

Dreamcast GPU uses PowerVR texture compression (PVRTC). You’ll need:

1. **PowerVR Texture Tool** (free from Imagination Technologies)
- <https://www.imaginationtech.com/>
- Download: “PowerVR SDK” tools
1. **Batch Conversion Script:**

```bash
#!/bin/bash
# Convert all DDS to TGA, then to PVRTC

for dds_file in *.dds; do
    # Step 1: DDS to TGA
    texconv -f R8G8B8A8_UNORM "$dds_file" -o "${dds_file%.dds}.tga"
    
    # Step 2: TGA to PVRTC (2bpp for Dreamcast)
    PVRTexTool -i "${dds_file%.dds}.tga" \
               -o "${dds_file%.dds}.pvr" \
               -f PVRTC1_2 \
               -q pvrtcbest
done
```

-----

## Part 5: Model Format Analysis

### Determining the `.mdl` Format

The source code doesn’t explicitly define the model format. You’ll need to reverse-engineer it:

**Method 1: Search for Model Loading Code**

```bash
cd /path/to/mgs2-source
grep -r "\.mdl\|LoadModel\|mesh\|geometry" bp/Source/Renderer/ \
  --include="*.cpp" --include="*.h" | head -50
```

**Method 2: File Header Analysis**

```bash
# Extract a known model file from the game
hexdump -C model.mdl | head -30

# Look for patterns:
# Common 3D format signatures:
# 00 00 80 3F = float 1.0 (often vertex data)
# Vertex count/face count usually near start
# Look for repeating patterns (vertex data)
```

**Method 3: Use Asset Extraction Tools**

Popular tools that support multiple unknown formats:

- **Noesis** (by Richwhitehouse) - Supports 100+ 3D formats
  - Download: <https://richwhitehouse.com/index.php?content=inc_projects.php&showproject=91>
  - Can batch export to FBX/OBJ
- **3D Ripper DX** - Captures 3D geometry from DirectX games
  - Can “rip” models directly from running game
  - Exports to OBJ/FBX

**Process Using Noesis:**

```
1. Open Noesis
2. File → Open → Select model.mdl
3. If format recognized: view in preview
4. Export to → FBX or OBJ format
5. Open in Blender for editing/re-exporting
```

-----

## Part 6: Audio Extraction & Conversion

### Audio Files

Source code shows `.wav` and `.adx` formats:

**WAV Files:**

- Standard uncompressed audio
- Can be used directly or re-encoded

**ADX Files:**

- Proprietary Dreamcast/console audio compression
- Used for streaming audio
- Need decoder

**Tools:**

```bash
# WAV to raw or OGG (for KallistiOS):
ffmpeg -i audio.wav -acodec libvorbis audio.ogg

# Or use SoX (cross-platform):
sox audio.wav audio.ogg

# For ADX files:
# Tool: adx2wav converter (available on GitHub)
# OR: ffmpeg (supports ADX):
ffmpeg -i audio.adx audio.wav
```

-----

## Part 7: Complete Extraction Workflow

### Recommended Step-by-Step Process

**Phase 1: Preparation**

```bash
# 1. Obtain PC version of MGS2 (legally)
# 2. Install game or mount ISO
# 3. Create working directory:
mkdir mgs2_asset_extraction
cd mgs2_asset_extraction
```

**Phase 2: Archive Extraction**

```bash
# 2. Locate game assets folder
# 3. Copy to extraction folder
cp -r "/path/to/game/data/*" ./raw_assets/

# 4. Identify archive types
file raw_assets/* | grep -i "archive\|compress"
```

**Phase 3: Decompress**

```bash
# If PSARC or proprietary format, use QuickBMS
quickbms script.bms raw_assets/data.psarc extracted/

# If standard ZIP/7z:
unzip -r raw_assets/*.zip -d extracted/
```

**Phase 4: Convert Formats**

```bash
# Create conversion scripts
./convert_textures.sh      # DDS → TGA → PVRTC
./convert_models.sh        # MDL → OBJ/FBX
./convert_audio.sh         # WAV → OGG (or native)
```

**Phase 5: Organize for Dreamcast Build**

```
dreamcast_assets/
├── textures/
│   └── *.pvr  (PowerVR format)
├── models/
│   └── *.mdl  (converted format)
├── audio/
│   └── *.wav or *.ogg
└── metadata/
    └── asset_list.txt (catalogs all files)
```

-----

## Part 8: Critical Notes

### Legal Considerations

- Extracting assets from a game you own is generally legal for personal use
- Redistributing extracted assets is illegal
- This POC is for research/educational purposes only

### Quality Degradation

- **PS2 extraction**: 2x compression loss (PS2 format → standard format)
- **PC extraction**: 1x compression loss (PC format → Dreamcast format)
- PC version is ~2x better quality starting point

### Time Investment

- Full asset extraction: 2-4 hours
- Format identification: 1-2 hours (if formats unknown)
- Conversion/optimization: 4-8 hours
- Total: Plan for ~1 day of work

### Common Pitfalls

1. **Wrong byte order** (endianness) - PS2 is big-endian, PC is little-endian
1. **Encrypted assets** - Some game data is encrypted (requires keys)
1. **Fragmented storage** - Assets split across multiple files
1. **Version-specific formats** - Different game versions may use different formats

-----

## Part 9: Quick Reference Command Index

```bash
# File inspection
file <filename>                    # Show file type
hexdump -C <filename> | head -20  # Show hex bytes

# Archive extraction
unzip -r archive.zip -d output/
7z x archive.7z -ooutput/

# Texture conversion
texconv -f R8G8B8A8_UNORM in.dds -o out.tga

# Audio conversion
ffmpeg -i in.wav out.ogg
sox in.wav out.ogg

# QuickBMS usage
quickbms script.bms input.psarc output_folder/

# Directory operations
find . -name "*.dds" -exec convert {} {}.png \;  # Batch process
```

-----

## Where to Get Help

1. **QuickBMS Scripts**: <https://aluigi.altervista.org/bms/>
1. **Game Format Database**: <https://xentax.com/>
1. **Asset Reversal Community**: Zenhax forums
1. **Specific MGS Tools**: GitHub search “Metal Gear Solid extraction”

This should give you everything you need to extract assets from the retail PC version and prepare them for Dreamcast conversion.