#pragma once

#include <cstdint>
#include <cstddef>

// PPC memory layout constants (from PE image analysis)
constexpr uint64_t PPC_MEM_IMAGE_BASE = 0x82000000ULL;
constexpr uint64_t PPC_MEM_IMAGE_SIZE = 0x3E0000ULL;    // 4,063,232 bytes
constexpr uint64_t PPC_MEM_CODE_BASE  = 0x820A0000ULL;  // .text section VA
constexpr uint64_t PPC_MEM_CODE_SIZE  = 0x237350ULL;    // .text section VSize
constexpr uint64_t PPC_MEM_TOTAL_SIZE = 0x100000000ULL; // 4 GB address space

// XEX entry point (from XEX2 header)
constexpr uint32_t PPC_ENTRY_POINT = 0x8214DB50;

// Stack configuration
constexpr uint32_t PPC_STACK_SIZE = 1 * 1024 * 1024;  // 1 MB stack
constexpr uint32_t PPC_STACK_BASE = 0x90000000;         // Stack top (grows down)

// Heap region for kernel stub allocations
constexpr uint32_t PPC_HEAP_BASE = 0xA0000000;
constexpr uint32_t PPC_HEAP_SIZE = 0x10000000;          // 256 MB

// Fake Xbox 360 kernel structures (KPCR / KTHREAD)
constexpr uint32_t PPC_KPCR_BASE    = 0x92000000;
constexpr uint32_t PPC_KPCR_SIZE    = 0x1000;           // 4 KB
constexpr uint32_t PPC_KTHREAD_BASE = 0x92001000;
constexpr uint32_t PPC_KTHREAD_SIZE = 0x1000;           // 4 KB

// Function lookup table
constexpr uint64_t PPC_FUNC_TABLE_OFFSET = PPC_MEM_IMAGE_BASE + PPC_MEM_IMAGE_SIZE;
constexpr uint64_t PPC_FUNC_TABLE_SIZE   = PPC_MEM_CODE_SIZE * 2;

// Allocate the PPC memory space using platform virtual memory.
uint8_t* ppc_memory_alloc();

// Free the PPC memory space.
void ppc_memory_free(uint8_t* base);

// Populate the function lookup table from PPCFuncMappings[].
void ppc_populate_func_table(uint8_t* base);

// Register a dynamic stub at a specific PPC address in the function table.
void ppc_register_dynamic_stub(uint8_t* base, uint32_t ppc_addr);

// Address reserved for the universal dynamic stub function.
// Last aligned address in code range: CODE_BASE + CODE_SIZE rounded up to 4
constexpr uint32_t PPC_DYNAMIC_STUB_ADDR = 0x822D7350;

// Global window handle (created in main.cpp, used by kernel_stubs.cpp)
#ifdef _WIN32
#include <windows.h>
extern HWND g_hwnd;
#endif
