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

static LONG CALLBACK NullPageHandler(EXCEPTION_POINTERS* ep) {
    auto* ctx = ep->ContextRecord;
    auto* rec = ep->ExceptionRecord;
    if (rec->ExceptionCode != STATUS_ACCESS_VIOLATION) return EXCEPTION_CONTINUE_SEARCH;
    if (rec->ExceptionInformation[0] != 0) return EXCEPTION_CONTINUE_SEARCH;
    uint64_t addr = rec->ExceptionInformation[1];
    uint64_t base = ctx->Rsi;
    if (base >= 0x100000000ULL && base <= 0x200000000ULL &&
        addr >= base && addr < base + 0x10000) {
        uint8_t* rip = (uint8_t*)ctx->Rip;
        int rex = 0, oplen = 0;
        uint8_t op = rip[0];
        if ((op & 0xF0) == 0x40) { rex = op; op = rip[1]; oplen = 1; }
        if (op == 0x8B) {
            uint8_t modrm = rip[oplen + 1];
            int reg = (modrm >> 3) & 7;
            if (rex & 0x04) reg += 8;
            int mod = (modrm >> 6) & 3;
            int rm = modrm & 7;
            int insn_len = oplen + 2;
            if (rm == 4 && mod != 3) insn_len++;
            if (mod == 0 && rm == 5) insn_len += 4;
            else if (mod == 1) insn_len += 1;
            else if (mod == 2) insn_len += 4;
            uint64_t* regs[] = { &ctx->Rax, &ctx->Rcx, &ctx->Rdx, &ctx->Rbx,
                                  &ctx->Rsp, &ctx->Rbp, &ctx->Rsi, &ctx->Rdi,
                                  &ctx->R8, &ctx->R9, &ctx->R10, &ctx->R11,
                                  &ctx->R12, &ctx->R13, &ctx->R14, &ctx->R15 };
            if (reg < 16) *regs[reg] = 0;
            ctx->Rip += insn_len;
            return EXCEPTION_CONTINUE_EXECUTION;
        }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

static LONG WINAPI CrashHandler(EXCEPTION_POINTERS* ep) {
    auto* ctx = ep->ContextRecord;
    auto* rec = ep->ExceptionRecord;
    fprintf(stderr, "\n========== CRASH ==========\n");
    fprintf(stderr, "Exception: 0x%08lX at RIP=0x%016llX\n",
            rec->ExceptionCode, (unsigned long long)ctx->Rip);
    fprintf(stderr, "===========================\n");
    fflush(stderr);
    return EXCEPTION_CONTINUE_SEARCH;
}
#endif

int main(int argc, char** argv) {
#ifdef _WIN32
    AddVectoredExceptionHandler(1, NullPageHandler);
    SetUnhandledExceptionFilter(CrashHandler);
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
