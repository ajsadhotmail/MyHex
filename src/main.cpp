#include "opts.hpp"
#include "term.hpp"
#include "viewer.hpp"
#include "dump.hpp"
#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[]) {
    // Enable ANSI on Windows before anything else (so --help output colors work).
    // On POSIX, ANSI is supported natively; term::init() is called by Viewer::run().
#ifdef _WIN32
    term::init();
#endif

    try {
        const auto o = parse(argc, argv);

        if (o.dump)
            run_dump(o);
        else
            Viewer(o).run();

    } catch (const std::exception& e) {
        term::restore();
        term::cursor_show();
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
