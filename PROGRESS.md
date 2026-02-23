# The Simpsons Arcade Recomp - Progress Log

## Phase Overview

| Phase | Description | Status |
|-------|-------------|--------|
| 1. Setup | Repo, toolchain, documentation | COMPLETE |
| 2. Analysis | XenonAnalyse, ABI address hunting | COMPLETE |
| 3. Configuration | TOML config, function definitions | COMPLETE |
| 4. Initial Recomp | First XenonRecomp pass, fix errors | IN PROGRESS |
| 5. Runtime Skeleton | Minimal runtime to link & boot | NOT STARTED |
| 6. Graphics | Xenos -> D3D12/Vulkan rendering | NOT STARTED |
| 7. Audio | Audio system implementation | NOT STARTED |
| 8. Input | Controller/keyboard input | NOT STARTED |
| 9. Integration | Full game loop, menus, gameplay | NOT STARTED |
| 10. Polish | Optimizations, bug fixes, testing | NOT STARTED |

---

## Detailed Log

### 2026-02-23 - Project Kickoff & First Recomp Pass

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

**Phase 4 - Initial Recomp (IN PROGRESS):**
- [x] Built XenonRecomp (clang-cl 21.1.8 + MSVC 2022, Ninja)
- [x] Applied Altivec/VMX patch from vig8
- [x] Ran XenonAnalyse (0 switch tables found)
- [x] **First XenonRecomp pass complete!**
  - 15,237 recompiled functions across 59 source files (~45MB)
  - 1,455 unrecognized instructions across 22 instruction types
  - Generated: ppc/ppc_recomp.{0..58}.cpp, ppc_func_mapping.cpp, ppc_recomp_shared.h
- [x] Updated generated/sources.cmake with all 59 recomp files
- [x] Created ppc/ppc_detail.h for safe PPC_CALL_INDIRECT_FUNC override
- [x] Updated project/CMakeLists.txt (include paths, PPC_INCLUDE_DETAIL)
- [ ] Add missing PPC instructions to XenonRecomp (22 types, 1455 instances)
- [ ] Re-run XenonRecomp after instruction patches

**Unrecognized PPC Instructions (22 types, 1455 total):**

| Instruction | Count | Category |
|-------------|-------|----------|
| sthu | 287 | Store halfword with update |
| stfsu | 247 | Store float single with update |
| lhzu | 227 | Load halfword unsigned with update |
| lfsu | 185 | Load float single with update |
| bdzf | 140 | Branch decrement CTR, false |
| lbzux | 67 | Load byte zero-extend with update indexed |
| lfsux | 45 | Load float single with update indexed |
| lhzux | 41 | Load halfword unsigned with update indexed |
| subfze | 33 | Subtract from zero extended |
| lhau | 33 | Load halfword algebraic with update |
| stdux | 28 | Store doubleword with update indexed |
| addc | 26 | Add carrying |
| addme | 25 | Add to minus one extended |
| stbux | 22 | Store byte with update indexed |
| ldux | 22 | Load doubleword with update indexed |
| lwzux | 8 | Load word zero-extend with update indexed |
| subfze. | 4 | Subtract from zero extended (CR) |
| stfdu | 4 | Store float double with update |
| lvehx | 4 | Load vector element halfword indexed |
| sthux | 3 | Store halfword with update indexed |
| lfdu | 2 | Load float double with update |
| bdnzt | 2 | Branch decrement CTR, nonzero true |

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

## Next Steps

1. **Add missing PPC instructions to XenonRecomp** (patch recompiler.cpp)
   - Update-form load/store (lhzu, sthu, stfsu, lfsu, etc.) - highest priority
   - Conditional branch variants (bdzf, bdnzt)
   - Integer arithmetic (subfze, addc, addme)
   - Vector element loads (lvehx)
2. Re-run XenonRecomp with patched toolchain
3. Build ReXGlue SDK
4. Attempt first compile of the recomp project
5. Build runtime skeleton (memory mapping, minimal stubs)

---

## Open Questions

1. Why did XenonAnalyse find 0 switch tables? (unusual for a game binary)
2. What engine does The Simpsons Arcade XBLA use?
3. Are there title update patches that change the binary?
4. The .SR files (SIMPSONS.SR, SIMPSONS_FW.SR) - what format are these? (likely game data archives)
