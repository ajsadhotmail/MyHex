#include "opts.hpp"
#include <iostream>
#include <stdexcept>
#include <string_view>

[[noreturn]] void help_exit(const char* prog, int rc) {
    auto& out = (rc == 0) ? std::cout : std::cerr;
    out << "Usage: " << prog << " [options] <file>\n\n"
           "  -n, --cols <N>    Bytes per row (default 16, max 64)\n"
           "  -s, --skip <N>    Skip N bytes from start\n"
           "  -l, --length <N>  Display at most N bytes\n"
           "  -d, --dump        Non-interactive output (pipe-friendly)\n"
           "  -C, --color       ANSI color (dump mode only; TUI always colors)\n"
           "      --no-ascii    Hide ASCII panel\n"
           "      --lower       Lowercase hex digits\n"
           "  -h, --help        This help\n\n"
           "TUI keys:\n"
           "  Up/Down   — scroll one row\n"
           "  PgUp/PgDn — scroll one page\n"
           "  Home/End  — first / last row\n"
           "  g         — goto hex offset\n"
           "  q / Esc   — quit\n";
    std::exit(rc);
}

Opts parse(int argc, char* argv[]) {
    if (argc < 2) help_exit(argv[0], 1);

    Opts o;
    for (int i = 1; i < argc; ++i) {
        std::string_view a = argv[i];

        auto nxt = [&]() -> const char* {
            if (i + 1 >= argc)
                throw std::runtime_error("'" + std::string(a) + "' requires an argument");
            return argv[++i];
        };

        if      (a=="-h"||a=="--help")   help_exit(argv[0], 0);
        else if (a=="-n"||a=="--cols") {
            auto v = std::stoul(nxt());
            if (!v || v > 64) throw std::runtime_error("--cols: must be 1-64");
            o.cols = v;
        }
        else if (a=="-s"||a=="--skip")   o.skip  = std::stoul(nxt());
        else if (a=="-l"||a=="--length") o.limit  = std::stoul(nxt());
        else if (a=="-d"||a=="--dump")   o.dump   = true;
        else if (a=="-C"||a=="--color")  o.color  = true;
        else if (a=="--no-ascii")        o.ascii  = false;
        else if (a=="--lower")           o.upper  = false;
        else if (!a.empty() && a[0]=='-')
            throw std::runtime_error("unknown option: " + std::string(a));
        else {
            if (!o.path.empty())
                throw std::runtime_error("multiple files specified");
            o.path = std::string(a);
        }
    }

    if (o.path.empty()) throw std::runtime_error("no file specified");
    return o;
}
