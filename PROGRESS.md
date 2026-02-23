# The Simpsons Arcade Recomp - Progress Log

## Phase Overview

| Phase | Description | Status |
|-------|-------------|--------|
| 1. Setup | Repo, toolchain, documentation | IN PROGRESS |
| 2. Analysis | XenonAnalyse, ABI address hunting | NOT STARTED |
| 3. Configuration | TOML config, function definitions | NOT STARTED |
| 4. Initial Recomp | First XenonRecomp pass, fix errors | NOT STARTED |
| 5. Runtime Skeleton | Minimal runtime to link & boot | NOT STARTED |
| 6. Graphics | Xenos -> D3D12/Vulkan rendering | NOT STARTED |
| 7. Audio | Audio system implementation | NOT STARTED |
| 8. Input | Controller/keyboard input | NOT STARTED |
| 9. Integration | Full game loop, menus, gameplay | NOT STARTED |
| 10. Polish | Optimizations, bug fixes, testing | NOT STARTED |

---

## Detailed Log

### 2026-02-23 - Project Kickoff

**Completed:**
- [x] Initialized repository structure (config/, docs/, tools/, src/, project/)
- [x] Created .gitignore (excludes binaries, build artifacts, toolchain)
- [x] Created project documentation (README.md, PROGRESS.md, docs/)
- [x] Created placeholder configurations for XenonRecomp
- [x] Created runtime source scaffolding adapted from vig8 project

**Game Details:**
- **Title:** The Simpsons Arcade
- **Title ID:** 0x584111FA
- **Developer:** Backbone Entertainment / Konami
- **Release:** February 2012 (Xbox Live Arcade)
- **Source:** STFS LIVE package (~18MB)
- **STFS Path:** `The Simpsons Arcade\584111FA\000D0000\5BA2C88EDCCD952114B3809DA53200C5C3B2236358`

**Build Environment:**
- Windows 11 Pro
- TODO: Install Python 3, Clang 18+, CMake, Ninja

---

## Next Steps

1. Install build prerequisites (Python 3, Clang 18+, CMake, Ninja)
2. Extract XEX from STFS container using `tools/extract_stfs.py`
3. Clone and build XenonRecomp
4. Run XenonAnalyse to detect jump tables
5. Locate ABI register save/restore addresses using `tools/find_abi_addrs.py`
6. Create XenonRecomp TOML configuration with found addresses
7. Run first recompilation pass
8. Fix compilation errors (missing instructions, switch boundaries)
9. Build runtime skeleton

---

## Open Questions

1. What engine does The Simpsons Arcade XBLA use? (likely custom or modified from original arcade)
2. Does it have any title update patches?
3. What is the XEX binary size and code complexity?
4. Are there known modding/reverse engineering resources for this game?
