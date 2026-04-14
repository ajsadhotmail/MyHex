#include "render.hpp"
#include "util.hpp"
#include <iostream>
#include <cstdio>

// Single definitions of the CC:: color constants
namespace CC {
    const char* const RST  = "\033[0m";
    const char* const ADDR = "\033[1;33m";
    const char* const DIM  = "\033[0;90m";
    const char* const CTRL = "\033[0;31m";
    const char* const PRNT = "\033[0;37m";
    const char* const HIGH = "\033[0;36m";
    const char* const ACHR = "\033[0;32m";
    const char* const BAR  = "\033[1;37;44m";
} // namespace CC

bool is_print(uint8_t b) { return b >= 0x20 && b <= 0x7E; }

std::size_t row_w(std::size_t cols, bool ascii) {
    // 8 (addr) + 2 (gap) + cols*2 (hex digits) + (cols-1) (spaces between)
    // + optional: 2 (gap) + 1 (|) + cols (chars) + 1 (|)
    return 10 + cols * 3 - 1 + (ascii ? cols + 4 : 0);
}

long long get_fsize(const std::string& path) {
    std::FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) return -1;
    std::fseek(f, 0, SEEK_END);
    long long sz = std::ftell(f);
    std::fclose(f);
    return sz;
}

std::string ansi_byte(uint8_t b, bool upper) {
    auto s = sfmt(upper ? "%02X" : "%02x", static_cast<unsigned>(b));
    if      (b == 0x00)              return std::string(CC::DIM)  + s + CC::RST;
    else if (b < 0x20 || b == 0x7F) return std::string(CC::CTRL) + s + CC::RST;
    else if (b >= 0x80)             return std::string(CC::HIGH)  + s + CC::RST;
    else                            return std::string(CC::PRNT)  + s + CC::RST;
}

std::string ansi_char(uint8_t b) {
    if (is_print(b))
        return std::string(CC::ACHR) + static_cast<char>(b) + CC::RST;
    return std::string(CC::DIM) + "." + CC::RST;
}

void print_header(std::size_t cols, bool ascii, bool upper) {
    std::cout << CC::ADDR << "  Offset  " << CC::RST;
    for (std::size_t i = 0; i < cols; ++i) {
        std::cout << CC::DIM
                  << sfmt(upper ? "%02X" : "%02x", static_cast<unsigned>(i))
                  << CC::RST;
        if (i + 1 < cols) std::cout << ' ';
    }
    if (ascii) {
        std::cout << CC::DIM << "  |";
        for (std::size_t i = 0; i < cols; ++i)
            std::cout << sfmt("%X", i % 16);
        std::cout << "|" << CC::RST;
    }
    // No trailing '\n' — caller decides
}

void print_row(std::size_t offset, const uint8_t* data, std::size_t n,
               std::size_t cols, bool ascii, bool upper) {
    std::cout << CC::ADDR
              << sfmt(upper ? "%08X" : "%08x", static_cast<unsigned>(offset))
              << CC::RST
              << CC::DIM << "  " << CC::RST;

    for (std::size_t i = 0; i < cols; ++i) {
        if (i < n) std::cout << ansi_byte(data[i], upper);
        else       std::cout << "  ";            // padding for partial last row
        if (i + 1 < cols) std::cout << ' ';     // space between bytes, not after last
    }

    if (ascii) {
        std::cout << CC::DIM << "  |" << CC::RST;
        for (std::size_t i = 0; i < n; ++i)    std::cout << ansi_char(data[i]);
        for (std::size_t i = n; i < cols; ++i) std::cout << ' ';
        std::cout << CC::DIM << "|" << CC::RST;
    }
    // No trailing '\n' — caller decides
}
