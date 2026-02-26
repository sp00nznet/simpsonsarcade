// Missing kernel stubs for The Simpsons Arcade
// These are Xbox 360 APIs not yet implemented in the ReXGlue SDK.

#include "simpsons_config.h"
#include "simpsons_settings.h"
#include <rex/runtime/guest/context.h>
#include <rex/runtime/guest/memory.h>

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

// XAM UI stubs
SIMPSONS_STUB(__imp__XamShowGamerCardUIForXUID)
SIMPSONS_STUB(__imp__XamShowAchievementsUI)
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
// sub_820CEC08 processes enumerated achievement data.
// It iterates entries (36-byte stride), checks [entry+32] & 0x20000 (ACHIEVED
// flag), and for each achieved one, writes 1 to [this + entry[0] + 63].
// When g_simpsons_unlock_all is set, we skip the flag check entirely, which
// unlocks Cool Stuff, ROM versions, and all levels.
PPC_EXTERN_FUNC(sub_820CE738);
PPC_EXTERN_FUNC(sub_820C6A88);

extern "C" PPC_FUNC(sub_820CEC08) {
    uint32_t this_ptr = ctx.r3.u32;
    int32_t count = ctx.r4.s32;
    uint32_t achievements = PPC_LOAD_U32(this_ptr + 56);

    // Clear achievement state: memset(this+64, 0, 12)
    ctx.r3.s64 = this_ptr + 64;
    ctx.r4.s64 = 0;
    ctx.r5.s64 = 12;
    sub_820C6A88(ctx, base);

    // Process each achievement entry (36-byte stride)
    for (int32_t i = 0; i < count && achievements; i++) {
        uint32_t entry = achievements + i * 36;
        uint32_t flags = PPC_LOAD_U32(entry + 32);

        if (g_simpsons_unlock_all || (flags & 0x20000)) {
            uint32_t offset = PPC_LOAD_U32(entry);
            PPC_STORE_U8(this_ptr + offset + 63, 1);
        }
    }

    // Finalization
    ctx.r3.u64 = this_ptr;
    sub_820CE738(ctx, base);

    // Mark processing done
    PPC_STORE_U32(this_ptr + 8, 1);
}

// (entry point shim removed - using real _xstart from recompiled code)
