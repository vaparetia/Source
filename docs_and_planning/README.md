# MGS2 Dreamcast Port — Documentation & Planning

This folder contains all planning, research, and educational materials for the Metal Gear Solid 2 Dreamcast port project.

## Folder Structure

```
docs_and_planning/
├── README.md                          (this file)
├── 01_PROJECT_OVERVIEW.md             (project scope, goals, constraints)
├── 02_HARDWARE_COMPARISON.md          (console specs, technical deep dive)
├── 03_THEORETICAL_FEASIBILITY.md      (architecture analysis, bottlenecks)
├── 04_ASSET_EXTRACTION_GUIDE.md       (tools, processes, file formats)
├── 05_IMPLEMENTATION_ROADMAP.md       (phases, timeline, milestones)
├── 06_QUICK_REFERENCE.md              (commands, tools, resources)
└── research_notes/
    ├── codebase_structure.md          (how the MGS2 source is organized)
    ├── file_formats.md                (PSARC, DDS, MDL, WAV analysis)
    └── platform_analysis.md           (rendering backends, architecture)
```

## Quick Navigation

**Starting Point:**
- New to the project? Start with `01_PROJECT_OVERVIEW.md`

**Understanding Hardware:**
- Read `02_HARDWARE_COMPARISON.md` before making technical decisions

**Is This Feasible?**
- Check `03_THEORETICAL_FEASIBILITY.md` for realistic assessment

**Ready to Extract Assets?**
- Follow `04_ASSET_EXTRACTION_GUIDE.md` for step-by-step process

**Implementation Planning:**
- Use `05_IMPLEMENTATION_ROADMAP.md` to structure your work

**During Development:**
- Bookmark `06_QUICK_REFERENCE.md` for commands and tools

## Document Status

| Document | Status | Last Updated |
|----------|--------|--------------|
| PROJECT_OVERVIEW | ✓ Complete | May 22, 2026 |
| HARDWARE_COMPARISON | ✓ Complete | May 22, 2026 |
| THEORETICAL_FEASIBILITY | ✓ Complete | May 22, 2026 |
| ASSET_EXTRACTION_GUIDE | ✓ Complete | May 22, 2026 |
| IMPLEMENTATION_ROADMAP | ✓ Complete | May 22, 2026 |
| QUICK_REFERENCE | ✓ Complete | May 22, 2026 |

## Key Takeaways (TL;DR)

- **Project Goal**: Proof-of-concept playable MGS2 port to Dreamcast
- **Status**: Theoretically feasible, requires 3-4 months solo effort
- **Main Challenge**: Asset extraction and GPU optimization
- **Best Asset Source**: PC version (higher quality, easier extraction)
- **Biggest Bottleneck**: CPU performance optimization

## Important Notes

- This is research/educational documentation only
- Asset extraction from retail games is for personal use
- Distribution of extracted assets is illegal
- Dreamcast has 128 MB RAM and ~200 MHz CPU — optimization will be required

## How to Use This Documentation

1. **Planning Phase**: Read overview → hardware comparison → feasibility analysis
2. **Asset Phase**: Follow extraction guide, identify file formats, convert assets
3. **Development Phase**: Reference implementation roadmap and quick reference
4. **During Implementation**: Use research notes as technical reference

---

**Questions or updates needed?** This documentation should evolve as the project progresses. Update files as you learn new information.
