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
