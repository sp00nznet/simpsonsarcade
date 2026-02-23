#pragma once

#include "simpsons_config.h"
#include <rex/runtime/guest.h>
#include <rex/logging.h>  // For REX_FATAL on unresolved calls

// Override SDK's raw PPC_CALL_INDIRECT_FUNC with a safe version.
// NOTE: Re-apply after rexglue codegen regenerates this file.
#undef PPC_CALL_INDIRECT_FUNC
#define PPC_CALL_INDIRECT_FUNC(x) do { \
    uint32_t _target = (x); \
    if (_target == 0) { \
        static int _nc = 0; \
        if (++_nc <= 5) \
            fprintf(stderr, "[WARN] Indirect call to NULL (LR=0x%08X) — skipping\n", (uint32_t)ctx.lr); \
        ctx.r3.u32 = 0; \
        break; \
    } \
    if (_target < (uint32_t)PPC_CODE_BASE || _target >= (uint32_t)(PPC_CODE_BASE + PPC_CODE_SIZE)) { \
        static int _oor = 0; \
        if (++_oor <= 20) \
            fprintf(stderr, "[WARN] Indirect call to 0x%08X outside code range — LR=0x%08X, CTR=0x%08X\n", \
                    _target, (uint32_t)ctx.lr, ctx.ctr.u32); \
        ctx.r3.u32 = 0; \
        break; \
    } \
    PPCFunc* _fn = PPC_LOOKUP_FUNC(base, _target); \
    if (!_fn) { \
        static int _nf = 0; \
        if (++_nf <= 50) \
            fprintf(stderr, "[WARN] Indirect call to 0x%08X: no recompiled function — LR=0x%08X\n", \
                    _target, (uint32_t)ctx.lr); \
        ctx.r3.u32 = 0; \
        break; \
    } \
    _fn(ctx, base); \
} while(0)

// Override PPC_UNIMPLEMENTED to warn instead of throwing.
// cctph/cctpl/cctpm are thread priority hints (no-ops on x86-64).
#undef PPC_UNIMPLEMENTED
#define PPC_UNIMPLEMENTED(addr, opcode) do { \
    static int _unimp_count = 0; \
    if (++_unimp_count <= 5) \
        fprintf(stderr, "[WARN] Unimplemented PPC instruction '%s' at 0x%08X — treating as no-op\n", \
                opcode, (uint32_t)(addr)); \
} while(0)

using namespace rex::runtime::guest;

// TODO: PPC_EXTERN_IMPORT declarations will be added by rexglue codegen
// after the first XenonRecomp pass generates the function table.

// Placeholder function mapping table (empty until recompilation)
extern PPCFuncMapping PPCFuncMappings[];
