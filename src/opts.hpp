#pragma once
#include <string>
#include <optional>
#include <cstddef>

struct Opts {
    std::string path;
    std::size_t cols  = 16;   ///< bytes per row (1..64)
    std::size_t skip  = 0;    ///< skip N bytes from start
    std::optional<std::size_t> limit; ///< max bytes to display
    bool ascii = true;        ///< show ASCII panel
    bool upper = true;        ///< uppercase hex digits
    bool dump  = false;       ///< non-interactive output mode
    bool color = false;       ///< ANSI color in dump mode (TUI always colors)
};

/// Print usage to stdout (rc==0) or stderr (rc!=0), then exit.
[[noreturn]] void help_exit(const char* prog, int rc);

/// Parse command-line arguments.
/// Throws std::runtime_error on invalid input.
Opts parse(int argc, char* argv[]);
