#pragma once

// Safe PPC_CALL_INDIRECT_FUNC override.
// Included via ppc_config.h when PPC_INCLUDE_DETAIL is defined.
// Must be included BEFORE ppc_context.h so the #ifndef guard skips the default.

#include <cstdio>
#include <cstdint>

// Import declarations for __imp__* kernel stubs need extern "C" linkage
// to match the ReXGlue SDK's definitions. PPC_FUNC is expanded at use time
// (after ppc_context.h defines it), so this deferred expansion works.
#define PPC_EXTERN_IMPORT(x) extern "C" PPC_FUNC(x)

// Physical address host offset: SDK maps physical addresses >= 0xE0000000
// at +0x1000 to allow VEH-based MMIO interception on Windows.
#define PPC_PHYS_HOST_OFFSET(addr) (((uint32_t)(addr) >= 0xE0000000u) ? 0x1000u : 0u)

// Override load/store macros to match SDK layout:
// 1. (uint32_t) cast to prevent sign-extension of 32-bit PPC addresses
// 2. PPC_PHYS_HOST_OFFSET for physical address translation
#define PPC_LOAD_U8(x)   (*(volatile uint8_t*)(base + (uint32_t)(x) + PPC_PHYS_HOST_OFFSET(x)))
#define PPC_LOAD_U16(x)  __builtin_bswap16(*(volatile uint16_t*)(base + (uint32_t)(x) + PPC_PHYS_HOST_OFFSET(x)))
#define PPC_LOAD_U32(x)  __builtin_bswap32(*(volatile uint32_t*)(base + (uint32_t)(x) + PPC_PHYS_HOST_OFFSET(x)))
#define PPC_LOAD_U64(x)  __builtin_bswap64(*(volatile uint64_t*)(base + (uint32_t)(x) + PPC_PHYS_HOST_OFFSET(x)))
#define PPC_STORE_U8(x, y)  (*(volatile uint8_t*)(base + (uint32_t)(x) + PPC_PHYS_HOST_OFFSET(x)) = (y))
#define PPC_STORE_U16(x, y) (*(volatile uint16_t*)(base + (uint32_t)(x) + PPC_PHYS_HOST_OFFSET(x)) = __builtin_bswap16(y))
#define PPC_STORE_U32(x, y) (*(volatile uint32_t*)(base + (uint32_t)(x) + PPC_PHYS_HOST_OFFSET(x)) = __builtin_bswap32(y))
#define PPC_STORE_U64(x, y) (*(volatile uint64_t*)(base + (uint32_t)(x) + PPC_PHYS_HOST_OFFSET(x)) = __builtin_bswap64(y))

#define PPC_CALL_INDIRECT_FUNC(x) do { \
    uint32_t _target = (x); \
    if (_target == 0) { \
        static int _nc = 0; \
        if (++_nc <= 5) \
            fprintf(stderr, "[WARN] Indirect call to NULL (LR=0x%08X) -- skipping\n", (uint32_t)ctx.lr); \
        ctx.r3.u32 = 0; \
        break; \
    } \
    if (_target < (uint32_t)PPC_CODE_BASE || _target >= (uint32_t)(PPC_CODE_BASE + PPC_CODE_SIZE)) { \
        /* Import thunks are in image range but below code range. */ \
        /* Try to simulate the thunk by reading the branch target from guest memory. */ \
        if (_target >= (uint32_t)PPC_IMAGE_BASE && _target < (uint32_t)PPC_CODE_BASE) { \
            /* Import thunk: read PPC instructions to find the actual target. */ \
            /* Xbox 360 import stubs: lis r11,hi / lwz r12,lo(r11) / mtctr r12 / bctr */ \
            /* The IAT entry address = (hi << 16) | lo, stored in guest memory. */ \
            uint32_t insn0 = PPC_LOAD_U32(_target);      /* lis r11, X */ \
            uint32_t insn1 = PPC_LOAD_U32(_target + 4);  /* lwz r12, Y(r11) */ \
            uint16_t hi = insn0 & 0xFFFF; \
            int16_t lo = (int16_t)(insn1 & 0xFFFF); \
            uint32_t iat_addr = ((uint32_t)hi << 16) + lo; \
            uint32_t resolved = PPC_LOAD_U32(iat_addr); \
            { static int _dbg = 0; if (++_dbg <= 5) { \
                uint32_t i2 = PPC_LOAD_U32(_target + 8); \
                uint32_t i3 = PPC_LOAD_U32(_target + 12); \
                fprintf(stderr, "[THUNK] 0x%08X: [%08X %08X %08X %08X] -> IAT=0x%08X -> 0x%08X\n", \
                    _target, insn0, insn1, i2, i3, iat_addr, resolved); fflush(stderr); } } \
            if (resolved >= (uint32_t)PPC_CODE_BASE && resolved < (uint32_t)(PPC_CODE_BASE + PPC_CODE_SIZE)) { \
                PPCFunc* _fn = PPC_LOOKUP_FUNC(base, resolved); \
                if (_fn) { _fn(ctx, base); break; } \
            } \
            /* Fallback: try as SDK-registered function */ \
            static int _imp = 0; \
            if (++_imp <= 20) \
                fprintf(stderr, "[WARN] Import thunk 0x%08X -> IAT 0x%08X -> 0x%08X (unresolved) -- LR=0x%08X\n", \
                        _target, iat_addr, resolved, (uint32_t)ctx.lr); \
            ctx.r3.u32 = 0; \
            break; \
        } \
        static int _oor = 0; \
        if (++_oor <= 20) \
            fprintf(stderr, "[WARN] Indirect call to 0x%08X outside code range -- LR=0x%08X, CTR=0x%08X\n", \
                    _target, (uint32_t)ctx.lr, ctx.ctr.u32); \
        ctx.r3.u32 = 0; \
        break; \
    } \
    PPCFunc* _fn = PPC_LOOKUP_FUNC(base, _target); \
    if (!_fn) { \
        static int _nf = 0; \
        if (++_nf <= 50) \
            fprintf(stderr, "[WARN] Indirect call to 0x%08X: no recompiled function -- LR=0x%08X\n", \
                    _target, (uint32_t)ctx.lr); \
        ctx.r3.u32 = 0; \
        break; \
    } \
    _fn(ctx, base); \
} while(0)
