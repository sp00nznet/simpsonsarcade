// Minimal console test for ReXGlue Runtime initialization

#include "simpsons_config.h"
#include "simpsons_init.h"

#include <rex/runtime.h>
#include <rex/logging.h>
#include <rex/cvar.h>

#include <cstdio>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>

// Guest memory base (set after runtime->Setup())
static uint64_t g_guest_base = 0;
static uint64_t g_guest_end = 0;

// Commit guest virtual pages on demand when accessed but not yet committed.
// The SDK reserves the full 4GB guest address space at 'base', but only commits
// pages that are explicitly allocated. Some game code (GPU init, etc.) may access
// pages that should have been committed by an unimplemented API.
static LONG CALLBACK GuestPageCommitHandler(EXCEPTION_POINTERS* ep) {
    auto* rec = ep->ExceptionRecord;
    if (rec->ExceptionCode != STATUS_ACCESS_VIOLATION) return EXCEPTION_CONTINUE_SEARCH;
    uint64_t addr = rec->ExceptionInformation[1];
    // Check if it's in guest address space (dynamic range from runtime)
    if (addr < g_guest_base || addr >= g_guest_end) return EXCEPTION_CONTINUE_SEARCH;
    // Try to commit the faulting page
    void* page = (void*)(addr & ~0xFFFULL);
    void* result = VirtualAlloc(page, 0x1000, MEM_COMMIT, PAGE_READWRITE);
    if (result) {
        static int count = 0;
        if (++count <= 50) {
            uint32_t ppc_addr = (uint32_t)(addr - g_guest_base);
            fprintf(stderr, "[PAGECOMMIT] Committed page for PPC 0x%08X (host %p) %s\n",
                    ppc_addr, page,
                    rec->ExceptionInformation[0] == 0 ? "READ" : "WRITE");
            fflush(stderr);
        }
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    // VirtualAlloc failed - log it
    {
        static int fail_count = 0;
        if (++fail_count <= 10) {
            uint32_t ppc_addr = (uint32_t)(addr - g_guest_base);
            fprintf(stderr, "[PAGECOMMIT] FAILED to commit page for PPC 0x%08X (host %p, err=%lu)\n",
                    ppc_addr, page, GetLastError());
            fflush(stderr);
        }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

static LONG CALLBACK CrashVEH(EXCEPTION_POINTERS* ep) {
    auto* ctx = ep->ContextRecord;
    auto* rec = ep->ExceptionRecord;
    if (rec->ExceptionCode == EXCEPTION_BREAKPOINT ||
        rec->ExceptionCode == EXCEPTION_SINGLE_STEP ||
        rec->ExceptionCode == 0x406D1388) return EXCEPTION_CONTINUE_SEARCH;
    fprintf(stderr, "\n========== EXCEPTION ==========\n");
    fprintf(stderr, "Thread: %lu\n", GetCurrentThreadId());
    fprintf(stderr, "Exception: 0x%08lX at RIP=0x%016llX\n",
            rec->ExceptionCode, (unsigned long long)ctx->Rip);
    if (rec->ExceptionCode == STATUS_ACCESS_VIOLATION) {
        fprintf(stderr, "Access address: 0x%016llX (%s)\n",
                (unsigned long long)rec->ExceptionInformation[1],
                rec->ExceptionInformation[0] == 0 ? "READ" : "WRITE");
    }
    // Get module base address for RVA calculation
    HMODULE hMod = NULL;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                       (LPCSTR)ctx->Rip, &hMod);
    if (hMod) {
        fprintf(stderr, "Module base: 0x%016llX  RVA: 0x%08llX\n",
                (unsigned long long)hMod,
                (unsigned long long)(ctx->Rip - (uint64_t)hMod));
    }
    fprintf(stderr, "RAX=0x%016llX RBX=0x%016llX RCX=0x%016llX RDX=0x%016llX\n",
            ctx->Rax, ctx->Rbx, ctx->Rcx, ctx->Rdx);
    fprintf(stderr, "RSI=0x%016llX RDI=0x%016llX RSP=0x%016llX RBP=0x%016llX\n",
            ctx->Rsi, ctx->Rdi, ctx->Rsp, ctx->Rbp);
    fprintf(stderr, "R8 =0x%016llX R9 =0x%016llX R10=0x%016llX R11=0x%016llX\n",
            ctx->R8, ctx->R9, ctx->R10, ctx->R11);
    fprintf(stderr, "R12=0x%016llX R13=0x%016llX R14=0x%016llX R15=0x%016llX\n",
            ctx->R12, ctx->R13, ctx->R14, ctx->R15);
    // Dump bytes at RIP for instruction identification
    fprintf(stderr, "Bytes at RIP: ");
    __try {
        uint8_t* rip = (uint8_t*)ctx->Rip;
        for (int i = 0; i < 16; i++) fprintf(stderr, "%02X ", rip[i]);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        fprintf(stderr, "<unreadable>");
    }
    fprintf(stderr, "\n");
    // Stack dump (extended)
    __try {
        fprintf(stderr, "Stack (RSP) - 48 entries:\n");
        uint64_t* sp = (uint64_t*)ctx->Rsp;
        for (int i = 0; i < 48; i++) {
            __try {
                // Mark likely return addresses (in module range)
                uint64_t val = sp[i];
                bool likely_ret = (hMod && val >= (uint64_t)hMod && val < (uint64_t)hMod + 0x10000000);
                fprintf(stderr, "  [RSP+%03X] = 0x%016llX%s\n", i*8, val,
                        likely_ret ? "  <-- likely return addr" : "");
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                fprintf(stderr, "  [RSP+%03X] = <unreadable>\n", i*8);
            }
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {}
    fprintf(stderr, "================================\n");
    fflush(stderr);
    return EXCEPTION_CONTINUE_SEARCH;
}
#endif

int main(int argc, char** argv) {
#ifdef _WIN32
    AddVectoredExceptionHandler(0, CrashVEH);  // last priority, logs everything
    // NOTE: GuestPageCommitHandler registered AFTER runtime->Setup() so it runs
    // BEFORE the SDK's MMIO handler in the VEH LIFO chain.
#endif
    fprintf(stderr, "[test] Starting ReXGlue boot test (The Simpsons Arcade)...\n");
    fflush(stderr);

    rex::cvar::Init(argc, argv);

    auto log_config = rex::BuildLogConfig(nullptr, "trace", {});
    rex::InitLogging(log_config);

    std::filesystem::path game_dir;
    if (argc > 1) {
        game_dir = argv[1];
    } else {
        game_dir = "extracted";
    }

    fprintf(stderr, "[test] Game dir: %s\n", game_dir.string().c_str());
    fflush(stderr);

    auto runtime = std::make_unique<rex::Runtime>(game_dir);

    auto status = runtime->Setup(
        static_cast<uint32_t>(PPC_CODE_BASE),
        static_cast<uint32_t>(PPC_CODE_SIZE),
        static_cast<uint32_t>(PPC_IMAGE_BASE),
        static_cast<uint32_t>(PPC_IMAGE_SIZE),
        PPCFuncMappings);

    fprintf(stderr, "[test] Setup returned: 0x%08X\n", status);
    fflush(stderr);

    if (status != 0) {
        fprintf(stderr, "[test] Setup FAILED\n");
        return 1;
    }

#ifdef _WIN32
    // Set up guest address range from the actual runtime membase
    g_guest_base = (uint64_t)runtime->virtual_membase();
    g_guest_end = g_guest_base + 0x200000000ULL;  // 8GB: virtual (4GB) + physical (4GB)
    fprintf(stderr, "[test] virtual_membase = %p (guest range: 0x%llX - 0x%llX)\n",
            runtime->virtual_membase(),
            (unsigned long long)g_guest_base, (unsigned long long)g_guest_end);
    fflush(stderr);
    // Register with priority 0 (LAST) so the SDK's MMIO handler processes
    // GPU/XMA register faults first. Only unhandled faults reach our handler.
    AddVectoredExceptionHandler(0, GuestPageCommitHandler);
#endif

    status = runtime->LoadXexImage("game:\\default.xex");
    fprintf(stderr, "[test] LoadXexImage returned: 0x%08X\n", status);

    if (status != 0) {
        fprintf(stderr, "[test] LoadXexImage FAILED\n");
        return 1;
    }

    fprintf(stderr, "[test] Boot test PASSED!\n");

    auto thread = runtime->LaunchModule();
    if (thread) {
        fprintf(stderr, "[test] Module launched, waiting...\n");
        thread->Wait(0, 0, 0, nullptr);
    }

    fprintf(stderr, "[test] Done.\n");
    return 0;
}
