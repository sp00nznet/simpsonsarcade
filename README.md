# The Simpsons Arcade - Static Recompilation Project

A static recompilation of **The Simpsons Arcade** (Xbox 360 / XBLA) to native x86-64 PC using [XenonRecomp](https://github.com/hedge-dev/XenonRecomp).

## Project Goal

Translate the Xbox 360 PowerPC executable of The Simpsons Arcade into native C++ code that can be compiled and run on modern PCs, preserving the original game logic while replacing Xbox 360 system services with PC equivalents.

## What is Static Recompilation?

Unlike emulation (which interprets instructions at runtime), static recompilation translates the entire binary ahead of time into equivalent C++ source code. Each PowerPC instruction maps to C++ code operating on a CPU state struct. The result is a native executable that runs at full speed without an emulator.

## Project Status

See [PROGRESS.md](PROGRESS.md) for detailed progress tracking.

**Current Phase:** Initial scaffolding

| Milestone | Status |
|-----------|--------|
| Repository scaffolding | Done |
| XEX extraction & analysis | Not Started |
| ABI address detection | Not Started |
| Jump table analysis | Not Started |
| Switch case boundary fixes | Not Started |
| Clean recompilation | Not Started |
| Runtime skeleton & build system | Not Started |
| PE extraction & data section loading | Not Started |
| Xbox 360 kernel import stubs | Not Started |
| Graphics (Xenos -> D3D12/Vulkan) | Not Started |
| Audio / Input / Integration | Not Started |

## Game Information

- **Title:** The Simpsons Arcade
- **Title ID:** 0x584111FA
- **Developer:** Backbone Entertainment
- **Publisher:** Konami
- **Release:** February 2012 (XBLA)
- **Genre:** Beat 'em up arcade port (original 1991 Konami arcade)

## Repository Structure

```
simpsonsarcade/
├── config/                        # XenonRecomp configuration
│   ├── simpsons.toml              # Main recomp config (ABI addresses, manual function defs)
│   ├── simpsons_switch_tables.toml # Jump table definitions
│   └── simpsons_rexglue.toml      # ReXGlue configuration
├── docs/                          # Documentation and research notes
│   ├── xenonrecomp-workflow.md
│   └── binary-analysis.md
├── src/                           # Runtime implementation source code
│   ├── main.cpp                   # Entry point, Win32 window, PPC context setup
│   ├── memory.cpp/h               # 4GB PPC memory space (VirtualAlloc/mmap)
│   ├── xex_loader.cpp/h           # PE image data section loader
│   ├── kernel_stubs.cpp           # Xbox 360 kernel/XAM/system stubs
│   └── math_polyfill.cpp          # C23 math function polyfills for MSVC
├── tools/                         # Toolchain (gitignored, built locally)
│   ├── XenonRecomp/               # Patched XenonRecomp with Altivec/VMX extensions
│   ├── patches/                   # XenonRecomp source patches
│   ├── dump_pe.cpp                # PE image extractor (links against XenonUtils)
│   └── extract_pe.py              # Python XEX decryption tool (requires pycryptodome)
├── extracted/                     # Extracted game files (gitignored)
│   ├── default.xex                # Xbox 360 executable
│   ├── pe_image.bin               # Decrypted/decompressed PE image
│   └── data/                      # Game data files
├── ppc/                           # Generated C++ output (gitignored, reproducible)
├── build/                         # CMake build directory (gitignored)
├── CMakeLists.txt                 # Build system (Clang + Ninja)
├── .gitignore
├── PROGRESS.md                    # Detailed progress log
└── README.md                      # This file
```

## Prerequisites

- **CMake 3.20+**
- **Clang 18+** (required by XenonRecomp and generated code)
- **Ninja** (recommended build system)
- **Python 3.8+** with `pycryptodome` (for XEX decryption)
- **Git**
- A legally obtained copy of **The Simpsons Arcade** XEX

## Quick Start

```bash
# 1. Clone this repo
git clone https://github.com/sp00nznet/simpsonsarcade.git
cd simpsonsarcade

# 2. Place your extracted default.xex in extracted/default.xex

# 3. Clone and build XenonRecomp (with Altivec/VMX patches)
git clone --recursive https://github.com/hedge-dev/XenonRecomp.git tools/XenonRecomp
# Apply patches from tools/patches/ (see below)
cd tools/XenonRecomp
cmake -B build -G Ninja
cmake --build build --config Release
cd ../..

# 4. Extract PE image from XEX (decrypt + decompress)
# Option A: Using the C++ tool (faster, links against XenonUtils)
clang++ -std=c++20 -O2 -I tools/XenonRecomp/XenonUtils -I tools/XenonRecomp/thirdparty \
    tools/dump_pe.cpp tools/XenonRecomp/build/XenonUtils/XenonUtils.lib -o tools/dump_pe.exe
tools/dump_pe.exe extracted/default.xex extracted/pe_image.bin

# Option B: Using Python (requires pycryptodome)
pip install pycryptodome
python tools/extract_pe.py extracted/default.xex extracted/pe_image.bin

# 5. Run analysis and recompilation
cd config
../tools/XenonRecomp/build/XenonAnalyse/XenonAnalyse ../extracted/default.xex simpsons_switch_tables.toml
../tools/XenonRecomp/build/XenonRecomp/XenonRecomp simpsons.toml ../tools/XenonRecomp/XenonUtils/ppc_context.h
cd ..

# 6. Build the native executable
cmake -B build -G Ninja -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_RC_COMPILER=llvm-rc
cmake --build build

# 7. Run
./build/simpsons.exe
```

## STFS Package

The game is distributed as an STFS LIVE package:
- **Content Type:** 0x000D0000 (Xbox Live Arcade)
- **Title ID:** 0x584111FA
- **Package path:** `The Simpsons Arcade\584111FA\000D0000\5BA2C88EDCCD952114B3809DA53200C5C3B2236358`
- Extract using `tools/extract_stfs.py`

## Toolchain

- [XenonRecomp](https://github.com/hedge-dev/XenonRecomp) - Xbox 360 static recompiler
- [XenosRecomp](https://github.com/hedge-dev/XenosRecomp) - Xenos GPU shader recompiler (future)

## References

- [UnleashedRecomp](https://github.com/hedge-dev/UnleashedRecomp) - Reference project (Sonic Unleashed recomp)
- [vig8](https://github.com/sp00nznet/vig8) - Sister project (Vigilante 8 Arcade recomp)
- [N64: Recompiled](https://github.com/N64Recomp/N64Recomp) - Inspiration for the approach

## License

This project contains no copyrighted game assets. You must provide your own legally obtained copy of The Simpsons Arcade.
