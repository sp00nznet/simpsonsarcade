#include "memory.h"
#include "ppc_config.h"
#include "ppc_context.h"

#include <cstdio>
#include <cstring>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <sys/mman.h>
#endif

uint8_t* ppc_memory_alloc()
{
    uint8_t* base = nullptr;

#ifdef _WIN32
    base = static_cast<uint8_t*>(
        VirtualAlloc(nullptr, PPC_MEM_TOTAL_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    if (!base)
    {
        fprintf(stderr, "Failed to allocate 4 GB virtual address space (error %lu)\n", GetLastError());
        return nullptr;
    }

#else
    base = static_cast<uint8_t*>(
        mmap(nullptr, PPC_MEM_TOTAL_SIZE,
             PROT_READ | PROT_WRITE,
             MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE,
             -1, 0));
    if (base == MAP_FAILED)
    {
        perror("mmap failed");
        return nullptr;
    }
#endif

    printf("PPC memory allocated at %p (4 GB)\n", base);
    printf("  Image:   0x%08llX - 0x%08llX\n",
        (unsigned long long)PPC_MEM_IMAGE_BASE,
        (unsigned long long)(PPC_MEM_IMAGE_BASE + PPC_MEM_IMAGE_SIZE));
    printf("  Stack:   0x%08X - 0x%08X\n",
        PPC_STACK_BASE - PPC_STACK_SIZE, PPC_STACK_BASE);
    printf("  Heap:    0x%08X - 0x%08X\n",
        PPC_HEAP_BASE, PPC_HEAP_BASE + PPC_HEAP_SIZE);

    return base;
}

void ppc_memory_free(uint8_t* base)
{
    if (!base) return;

#ifdef _WIN32
    VirtualFree(base, 0, MEM_RELEASE);
#else
    munmap(base, PPC_MEM_TOTAL_SIZE);
#endif
}

void ppc_populate_func_table(uint8_t* base)
{
    size_t count = 0;
    for (const PPCFuncMapping* m = PPCFuncMappings; m->host != nullptr; ++m)
    {
        uint32_t guest_addr = static_cast<uint32_t>(m->guest);
        if (guest_addr >= PPC_CODE_BASE && guest_addr < PPC_CODE_BASE + PPC_CODE_SIZE)
        {
            uint64_t table_offset = PPC_FUNC_TABLE_OFFSET +
                (static_cast<uint64_t>(guest_addr - PPC_CODE_BASE) * 2);
            auto* slot = reinterpret_cast<PPCFunc**>(base + table_offset);
            *slot = m->host;
            ++count;
        }
    }

    printf("  Populated %zu function table entries\n", count);

    if (PPC_DYNAMIC_STUB_ADDR != 0)
        ppc_register_dynamic_stub(base, PPC_DYNAMIC_STUB_ADDR);
}

static void ppc_dynamic_stub_impl(PPCContext& __restrict ctx, uint8_t* base)
{
    static uint64_t call_count = 0;
    call_count++;
    if (call_count <= 10 || (call_count & 0xFFFF) == 0)
    {
        fprintf(stderr, "[DYN-STUB] Dynamic stub called (#%llu), LR=0x%08X, r3=0x%08X\n",
                (unsigned long long)call_count, (uint32_t)ctx.lr, ctx.r3.u32);
        fflush(stderr);
    }
    ctx.r3.u32 = 0;
}

void ppc_register_dynamic_stub(uint8_t* base, uint32_t ppc_addr)
{
    if (ppc_addr >= PPC_CODE_BASE && ppc_addr < PPC_CODE_BASE + PPC_CODE_SIZE)
    {
        uint64_t table_offset = PPC_FUNC_TABLE_OFFSET +
            (static_cast<uint64_t>(ppc_addr - PPC_CODE_BASE) * 2);
        auto* slot = reinterpret_cast<PPCFunc**>(base + table_offset);
        *slot = ppc_dynamic_stub_impl;
    }
}
