// Missing kernel stubs for The Simpsons Arcade
// These are Xbox 360 APIs not yet implemented in the ReXGlue SDK.

#include "simpsons_config.h"
#include <rex/runtime/guest/context.h>
#include <rex/runtime/guest/memory.h>

using namespace rex::runtime::guest;

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

// (entry point shim removed - using real _xstart from recompiled code)
