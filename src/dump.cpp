#include "dump.hpp"
#include "render.hpp"
#include "util.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cstdio>
#include <cstdint>

void run_dump(const Opts& o) {
    // Use C-style FILE* to avoid a GCC 8.x / MinGW bug where inlining
    // std::ifstream's constructor at -O1+ causes a stack corruption crash.
    std::FILE* fp = std::fopen(o.path.c_str(), "rb");
    if (!fp) throw std::runtime_error("File not found: " + o.path);

    // Measure file size via seek
    std::fseek(fp, 0, SEEK_END);
    const long long sz = std::ftell(fp);
    std::rewind(fp);

    if (o.skip > 0) {
        if (std::fseek(fp, static_cast<long>(o.skip), SEEK_SET) != 0) {
            std::fclose(fp);
            throw std::runtime_error("--skip exceeds file size");
        }
    }

    const std::string sep(row_w(o.cols, o.ascii), '-');

    // ── Header ───────────────────────────────────────────────────────────────

    if (o.color) {
        std::cout << CC::DIM << "File : " << CC::RST << o.path << '\n'
                  << CC::DIM << "Size : " << CC::RST << sz << " bytes\n\n";
        print_header(o.cols, o.ascii, o.upper);
        std::cout << '\n' << CC::DIM << sep << CC::RST << '\n';
    } else {
        std::cout << "File : " << o.path << '\n'
                  << "Size : " << sz << " bytes\n\n"
                  << "  Offset  ";
        for (std::size_t i = 0; i < o.cols; ++i) {
            std::cout << sfmt(o.upper ? "%02X" : "%02x", static_cast<unsigned>(i));
            if (i + 1 < o.cols) std::cout << ' ';
        }
        if (o.ascii) {
            std::cout << "  |";
            for (std::size_t i = 0; i < o.cols; ++i)
                std::cout << sfmt("%X", i % 16);
            std::cout << "|";
        }
        std::cout << '\n' << sep << '\n';
    }

    // ── Data rows ─────────────────────────────────────────────────────────────

    std::vector<uint8_t> buf(o.cols);
    std::size_t off = o.skip, total = 0;

    for (;;) {
        std::size_t want = o.cols;
        if (o.limit) {
            if (total >= *o.limit) break;
            want = std::min(want, *o.limit - total);
        }

        std::size_t n = std::fread(buf.data(), 1, want, fp);
        if (n == 0) break;

        if (o.color) {
            print_row(off, buf.data(), n, o.cols, o.ascii, o.upper);
            std::cout << '\n';
        } else {
            std::cout << sfmt(o.upper ? "%08X" : "%08x", static_cast<unsigned>(off))
                      << "  ";
            for (std::size_t i = 0; i < o.cols; ++i) {
                if (i < n)
                    std::cout << sfmt(o.upper ? "%02X" : "%02x",
                                      static_cast<unsigned>(buf[i]));
                else
                    std::cout << "  ";
                if (i + 1 < o.cols) std::cout << ' ';
            }
            if (o.ascii) {
                std::cout << "  |";
                for (std::size_t i = 0; i < n; ++i)
                    std::cout << static_cast<char>(is_print(buf[i]) ? buf[i] : '.');
                for (std::size_t i = n; i < o.cols; ++i) std::cout << ' ';
                std::cout << "|";
            }
            std::cout << '\n';
        }
        off += n; total += n;
    }

    std::fclose(fp);

    // ── Footer ────────────────────────────────────────────────────────────────

    const std::size_t rows = total / o.cols;
    const std::size_t rem  = total % o.cols;

    if (o.color) {
        std::cout << CC::DIM << sep << CC::RST << '\n'
                  << CC::ADDR << "Total" << CC::RST;
    } else {
        std::cout << sep << "\nTotal";
    }
    std::cout << sfmt(" : %zu bytes  (%zu row%s, %zu byte%s remainder)\n",
                      total,
                      rows, rows == 1 ? "" : "s",
                      rem,  rem  == 1 ? "" : "s");
}
