#pragma once

#include <cstdint>

// Load data sections from a pre-extracted PE image into PPC memory.
bool xex_load_data_sections(uint8_t* base, const char* pe_path);
