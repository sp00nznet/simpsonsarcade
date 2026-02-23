# XenonRecomp Workflow for The Simpsons Arcade

## Tool Overview

**XenonRecomp** (https://github.com/hedge-dev/XenonRecomp) is a static recompiler that converts Xbox 360 PowerPC executables into C++ source code compilable for x86-64.

### What it produces
- C++ source files with 1:1 translated PowerPC functions
- Each function operates on a `ppc_context` struct (32 GPRs, 32 FPRs, 128 vector regs, CR, CTR, XER, LR, MSR, FPSCR)
- Functions use weak linking for easy hooking
- Handles big-endian to little-endian byte-swapping automatically
- Converts jump tables to C++ switch statements
- Supports mid-ASM hooks for injecting custom code

### What it does NOT provide
- No runtime implementation (graphics, audio, input, I/O, memory, threading)
- No MMIO operations
- No exception handling support

## Step-by-Step Workflow

### Step 1: Build XenonRecomp

```bash
git clone --recursive https://github.com/hedge-dev/XenonRecomp.git tools/XenonRecomp
cd tools/XenonRecomp
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

**Requirements:** CMake 3.20+, Clang 18+, C++17

### Step 2: Run XenonAnalyse

```bash
XenonAnalyse "../extracted/default.xex" config/simpsons_switch_tables.toml
```

### Step 3: Locate ABI Addresses

Use `tools/find_abi_addrs.py` to scan the PE image for standard PowerPC ABI register save/restore functions.

### Step 4: Create TOML Configuration

```toml
[main]
file_path = "../extracted/default.xex"
out_directory_path = "../ppc"
switch_table_file_path = "simpsons_switch_tables.toml"

# ABI addresses (MUST be found in the actual binary)
savegprlr_14_address = 0x00000000  # TODO: find
restgprlr_14_address = 0x00000000  # TODO: find
savefpr_14_address = 0x00000000    # TODO: find
restfpr_14_address = 0x00000000    # TODO: find
savevmx_14_address = 0x00000000   # TODO: find
restvmx_14_address = 0x00000000   # TODO: find
savevmx_64_address = 0x00000000   # TODO: find
restvmx_64_address = 0x00000000   # TODO: find

[optimizations]
skip_lr = false
skip_msr = false
ctr_as_local = false
xer_as_local = false
reserved_as_local = false
cr_as_local = false
non_argument_as_local = false
non_volatile_as_local = false
```

### Step 5: Run XenonRecomp

```bash
mkdir -p ppc
XenonRecomp config/simpsons.toml tools/XenonRecomp/XenonUtils/ppc_context.h
```

### Step 6: Build Runtime (The Hard Part)

The generated C++ is just the translated game logic. To run the game, we need:

1. **Memory management** - Map Xbox 360 virtual address space
2. **Graphics** - Translate Xenos GPU draw calls to D3D12 or Vulkan
3. **Shaders** - Recompile Xenos shaders (XenosRecomp)
4. **Audio** - XMA/PCM audio decoding and playback
5. **Input** - Xbox 360 controller calls to PC input
6. **File I/O** - Xbox 360 file system to PC paths
7. **Threading** - Xbox 360 threading to Windows/POSIX threads
8. **System calls** - Xbox 360 kernel stubs (XAM, XEX loader, etc.)

### Step 7: Iterate

- Fix recompilation errors
- Add `functions` entries for broken function boundaries
- Add mid-ASM hooks where needed

### Step 8: Optimize

Enable TOML optimization flags one by one after everything works.

## Reference Projects

- [UnleashedRecomp](https://github.com/hedge-dev/UnleashedRecomp) - Sonic Unleashed recomp
- [vig8](https://github.com/sp00nznet/vig8) - Vigilante 8 Arcade recomp (sister project)
