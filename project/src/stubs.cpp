// Missing kernel stubs for The Simpsons Arcade
// These are Xbox 360 APIs not yet implemented in the ReXGlue SDK.

#include "simpsons_config.h"
#include "simpsons_settings.h"
#include <rex/runtime/guest/context.h>
#include <rex/runtime/guest/memory.h>
#include <cstdio>

using namespace rex::runtime::guest;

bool g_simpsons_unlock_all = true;

#define SIMPSONS_STUB_RETURN(name, val) \
    extern "C" PPC_FUNC(name) { (void)base; ctx.r3.u64 = (val); }

#define SIMPSONS_STUB(name) SIMPSONS_STUB_RETURN(name, 0)

// Networking stubs (NetDll_*)
SIMPSONS_STUB(__imp__NetDll_XNetUnregisterInAddr)
SIMPSONS_STUB(__imp__NetDll_XNetConnect)
SIMPSONS_STUB(__imp__NetDll_XNetGetConnectStatus)
SIMPSONS_STUB(__imp__NetDll_XNetQosLookup)
SIMPSONS_STUB(__imp__NetDll_WSAGetOverlappedResult)

// XAM UI stubs (with init trace)
extern "C" PPC_FUNC(__imp__XamShowAchievementsUI) {
    (void)base;
    FILE* f = fopen("unlock_trace.txt", "a");
    if (f) { fprintf(f, "[TRACE] XamShowAchievementsUI called\n"); fclose(f); }
    ctx.r3.u64 = 0;
}
SIMPSONS_STUB(__imp__XamShowGamerCardUIForXUID)
SIMPSONS_STUB(__imp__XamShowMarketplaceUI)
SIMPSONS_STUB_RETURN(__imp__XamUserCreateStatsEnumerator, 1)
SIMPSONS_STUB(__imp__XamVoiceSubmitPacket)

// Kernel memory allocation
SIMPSONS_STUB(__imp__ExAllocatePoolWithTag)

// USB Camera stubs (XUsbcam*)
SIMPSONS_STUB_RETURN(__imp__XUsbcamCreate, 1)
SIMPSONS_STUB(__imp__XUsbcamDestroy)
SIMPSONS_STUB(__imp__XUsbcamGetState)
SIMPSONS_STUB_RETURN(__imp__XUsbcamSetConfig, 1)
SIMPSONS_STUB_RETURN(__imp__XUsbcamSetView, 1)
SIMPSONS_STUB_RETURN(__imp__XUsbcamSetCaptureMode, 1)
SIMPSONS_STUB_RETURN(__imp__XUsbcamReadFrame, 1)

// ObReferenceObject - provided by rex::kernel via WHOLEARCHIVE

// XAM UI stubs not yet in SDK
SIMPSONS_STUB(__imp__XamShowFriendsUI)

// XAudio ducker stubs not yet in SDK (return sensible defaults)
SIMPSONS_STUB(__imp__XAudioGetDuckerLevel)
SIMPSONS_STUB(__imp__XAudioGetDuckerReleaseTime)
SIMPSONS_STUB(__imp__XAudioGetDuckerAttackTime)
SIMPSONS_STUB(__imp__XAudioGetDuckerHoldTime)
SIMPSONS_STUB(__imp__XAudioGetDuckerThreshold)

// ============================================================================
// Achievement unlock override
// ============================================================================
// sub_820CEAE0 is the achievement loading function. It:
//   1. Calls virtual functions to check profile/sign-in state
//   2. Creates achievement enumerator via XamUserCreateAchievementEnumerator
//   3. Calls XamEnumerate to fill buffer
//   4. Later, sub_820CEC08 processes the buffer, checking [entry+32] & 0x20000
//      (ACHIEVED flag) and writing 1 to [manager + achievement_id + 63]
//
// The profile checks fail without a real Xbox Live profile, so the entire
// pipeline never runs. When g_simpsons_unlock_all is set, we override
// sub_820CEAE0 to bypass all checks and directly write unlock bytes for
// all 33 achievement slots (IDs 1-33 → manager offsets 64-96).
PPC_EXTERN_FUNC(sub_820CE738);
PPC_EXTERN_FUNC(sub_820C6A88);

// Override the achievement LOADER to bypass profile/sign-in checks.
// Without this, the virtual function calls fail and nothing happens.
// NOTE: No extern "C" — must match C++ linkage of PPC_WEAK_FUNC alias.
PPC_FUNC(sub_820CEAE0) {
    uint32_t manager = ctx.r3.u32;

    // Reset enumerator state (same as original)
    sub_820CE738(ctx, base);

    // Clear achievement state: memset(manager+64, 0, 12)
    ctx.r3.s64 = manager + 64;
    ctx.r4.s64 = 0;
    ctx.r5.s64 = 12;
    sub_820C6A88(ctx, base);

    // Don't write unlock bytes here — sub_820CEC08 runs later and
    // would memset them back to 0. We handle it in that override instead.

    // Mark loading complete (not error)
    PPC_STORE_U32(manager + 8, 0);
}

// Override the achievement PROCESSOR. The original:
//   1. memset(manager+64, 0, 12)
//   2. For each achievement with ACHIEVED flag: manager[id + 63] = 1
//   3. Finalize + mark done
// When unlock_all is set, we skip the memset and force all 12 bytes to 1.
// Manager objects are 76 bytes (0x4C) apart, so only offsets 64-75 are safe.
PPC_FUNC(sub_820CEC08) {
    uint32_t manager = ctx.r3.u32;

    if (g_simpsons_unlock_all) {
        // Skip memset — directly write 1 to all 12 unlock slots
        for (uint32_t i = 0; i < 12; i++) {
            PPC_STORE_U8(manager + 64 + i, 1);
        }
    } else {
        // Clear then process normally (no achievements available)
        ctx.r3.s64 = manager + 64;
        ctx.r4.s64 = 0;
        ctx.r5.s64 = 12;
        sub_820C6A88(ctx, base);
    }

    // Finalization
    ctx.r3.u64 = manager;
    sub_820CE738(ctx, base);

    // Mark processing done
    PPC_STORE_U32(manager + 8, 1);
}

// (entry point shim removed - using real _xstart from recompiled code)
