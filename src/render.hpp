#pragma once
#include <cstdint>
#include <cstddef>
#include <string>

/// ANSI color escape codes used throughout the UI.
/// Declared extern here; defined once in render.cpp.
namespace CC {
    extern const char* const RST;   ///< reset all attributes
    extern const char* const ADDR;  ///< bold yellow  — addresses
    extern const char* const DIM;   ///< dark grey    — zero bytes / separators
    extern const char* const CTRL;  ///< red          — control bytes (< 0x20 or 0x7F)
    extern const char* const PRNT;  ///< white        — printable ASCII bytes
    extern const char* const HIGH;  ///< cyan         — high bytes (>= 0x80)
    extern const char* const ACHR;  ///< green        — ASCII panel printable chars
    extern const char* const BAR;   ///< bold white on blue — title / status bars
} // namespace CC

/// True for printable ASCII (0x20–0x7E).
bool is_print(uint8_t b);

/// Visual character width of one full data row (ANSI codes not counted).
std::size_t row_w(std::size_t cols, bool ascii);

/// File size in bytes, or -1 on error.
long long get_fsize(const std::string& path);

/// Colored hex representation of one byte ("XX" + ANSI codes).
std::string ansi_byte(uint8_t b, bool upper);

/// Colored ASCII representation for the right-hand panel (1 char + ANSI codes).
std::string ansi_char(uint8_t b);

/// Write the column-index header line to stdout.
/// Does NOT emit a trailing newline — caller decides.
void print_header(std::size_t cols, bool ascii, bool upper);

/// Write one hex+ASCII data row to stdout.
/// Does NOT emit a trailing newline — caller decides.
void print_row(std::size_t offset, const uint8_t* data, std::size_t n,
               std::size_t cols, bool ascii, bool upper);
