# The Simpsons Arcade Recomp - Progress Log

## Phase Overview

| Phase | Description | Status |
|-------|-------------|--------|
| 1. Setup | Repo, toolchain, documentation | COMPLETE |
| 2. Analysis | XenonAnalyse, ABI address hunting | COMPLETE |
| 3. Configuration | TOML config, function definitions | COMPLETE |
| 4. Initial Recomp | First XenonRecomp pass, fix errors | COMPLETE |
| 5. Runtime Skeleton | ReXGlue SDK build, first link | COMPLETE |
| 6. Graphics | Xenos -> D3D12/Vulkan rendering | NOT STARTED |
| 7. Audio | Audio system implementation | NOT STARTED |
| 8. Input | Controller/keyboard input | NOT STARTED |
| 9. Integration | Full game loop, menus, gameplay | NOT STARTED |
| 10. Polish | Optimizations, bug fixes, testing | NOT STARTED |

---

## Detailed Log

### 2026-02-23 - Project Kickoff through First Successful Build

**Phase 1 - Setup (COMPLETE):**
- [x] Initialized repository structure (config/, docs/, tools/, src/, project/)
- [x] Created .gitignore (excludes binaries, build artifacts, toolchain)
- [x] Created project documentation (README.md, PROGRESS.md, docs/)
- [x] Created runtime source scaffolding adapted from vig8 project
- [x] Created 205 Xbox 360 kernel stubs (src/kernel_stubs.cpp)
- [x] Ported critical upstream fixes from vig8 (crash diagnostics, NullPageHandler, init overrides)

**Phase 2 - Analysis (COMPLETE):**
- [x] Extracted 18 files from STFS LIVE package using tools/extract_stfs.py
- [x] Extracted PE image from XEX2 (LZX decompression, 4,063,232 bytes)
- [x] Found all 8 ABI register save/restore addresses
- [x] PE analysis: IMAGE_BASE=0x82000000, CODE_BASE=0x820A0000, CODE_SIZE=0x237350
- [x] Entry point: 0x8214DB50
- [x] Documented results in docs/binary-analysis.md

**Phase 3 - Configuration (COMPLETE):**
- [x] config/simpsons.toml with all ABI addresses and paths
- [x] config/simpsons_switch_tables.toml (XenonAnalyse found 0 jump tables)

**Phase 4 - Initial Recomp (COMPLETE):**
- [x] Built XenonRecomp (clang-cl 21.1.8 + MSVC 2022, Ninja)
- [x] Applied Altivec/VMX patch from vig8
- [x] Ran XenonAnalyse (0 switch tables found)
- [x] First XenonRecomp pass: 15,237 functions across 59 source files (~45MB)
- [x] Added 21 missing PPC instruction handlers to XenonRecomp
- [x] Re-ran XenonRecomp: **0 unrecognized instructions**
- [x] Created patch: tools/patches/xenonrecomp-missing-instructions.patch

**Phase 5 - Runtime Skeleton (COMPLETE):**
- [x] Built ReXGlue SDK (668 targets, clang/clang++ + vcvarsall)
  - Fixed 14 broken Git symlinks in libmspack (Windows symlink issue)
  - SDK installed to tools/rexglue-sdk/out/install/win-amd64/
- [x] Updated project/CMakeLists.txt for SDK integration
  - Added SIMDE include path (for PPC vector intrinsics)
  - Added PPC_INCLUDE_DETAIL compile definition
  - Added /WHOLEARCHIVE for rexkernel on Windows (kernel import stubs)
- [x] Fixed extern "C" linkage mismatch for __imp__* kernel imports
  - Added PPC_EXTERN_IMPORT macro in ppc/ppc_detail.h
  - Post-processed ppc_recomp_shared.h: PPC_EXTERN_FUNC → PPC_EXTERN_IMPORT for imports
- [x] Added 25 game-specific stubs in project/src/stubs.cpp
  - Networking (NetDll_XNetUnregisterInAddr, etc.)
  - XAM UI (XamShowFriendsUI, XamShowGamerCardUIForXUID, etc.)
  - XAudio ducker (XAudioGetDuckerLevel, etc.)
  - USB Camera (XUsbcam*)
  - Kernel pool (ExAllocatePoolWithTag)
- [x] **Both executables link successfully!**
  - simpsons.exe (18MB, windowed GUI)
  - simpsons_test.exe (18MB, console test)

**Game Details:**
- **Title:** The Simpsons Arcade
- **Title ID:** 0x584111FA
- **Developer:** Backbone Entertainment / Konami
- **Release:** February 2012 (Xbox Live Arcade)
- **Source:** STFS LIVE package (~18MB)
- **XEX Size:** 1,327,104 bytes (XEX2, LZX compressed)
- **PE Size:** 4,063,232 bytes (0x3E0000)
- **Code Size:** 0x237350 (2,323,280 bytes)
- **Functions:** 15,237 recompiled

**Build Environment:**
- Windows 11 Pro
- Python 3.13, Clang 21.1.8 (LLVM), MSVC 19.44, CMake 4.2.1, Ninja 1.13.2

---

## Key Technical Discoveries

1. **XenonRecomp + extern "C" mismatch:** The recompiler generates `PPC_EXTERN_FUNC` for all forward declarations, which uses C++ linkage. But the ReXGlue SDK's kernel stubs use `extern "C"` linkage. Required post-processing the shared header to use `PPC_EXTERN_IMPORT` for `__imp__*` imports.

2. **WHOLEARCHIVE required on Windows:** The Xbox 360 kernel stubs in `rexkernel.lib` are only referenced by the recompiled PPC code. Without `/WHOLEARCHIVE`, the static linker doesn't pull in the kernel implementation objects. Linux equivalent: `--whole-archive`.

3. **21 missing PPC instructions:** XenonRecomp didn't handle update-form loads/stores (lhzu, sthu, lfsu, etc.), conditional branch variants (bdzf, bdnzt), integer arithmetic with carry (addc, addme, subfze), and vector element loads (lvehx). Patch adds all 21.

4. **Git symlinks on Windows:** libmspack in the SDK uses symlinks that Git stores as plain text files on Windows. Required manual fixup by copying actual file content.

---

## Next Steps

1. **First runtime test** - Run simpsons_test.exe and observe behavior
2. **Memory mapping** - Ensure PE image is loaded at correct base address
3. **Graphics hookup** - Connect Xenos GPU commands to D3D12
4. **Audio hookup** - Connect XMA audio to native audio backend
5. **Input hookup** - Connect XInput to native input system
6. **Game data loading** - Mount .SR data archives

---

## Open Questions

1. Why did XenonAnalyse find 0 switch tables? (unusual for a game binary)
2. What engine does The Simpsons Arcade XBLA use?
3. Are there title update patches that change the binary?
4. The .SR files (SIMPSONS.SR, SIMPSONS_FW.SR) - what format are these? (likely game data archives)
