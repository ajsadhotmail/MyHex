#include "viewer.hpp"
#include "term.hpp"
#include "render.hpp"
#include "util.hpp"
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <cstring>

// ── Constructor / Destructor ──────────────────────────────────────────────────

Viewer::Viewer(const Opts& o) : fp_(nullptr), sz_(0), o_(o) {
    sz_ = get_fsize(o.path);
    if (sz_ < 0)
        throw std::runtime_error("File not found: " + o.path);
    if (static_cast<long long>(o.skip) >= sz_)
        throw std::runtime_error("--skip exceeds file size");

    // Use FILE* to avoid a GCC 8.x / MinGW bug with std::ifstream inlining at -O1+
    fp_ = std::fopen(o.path.c_str(), "rb");
    if (!fp_)
        throw std::runtime_error("Cannot open: " + o.path);
}

// ── Geometry ──────────────────────────────────────────────────────────────────

std::size_t Viewer::total_rows() const {
    long long avail = sz_ - static_cast<long long>(o_.skip);
    if (avail <= 0) return 0;
    std::size_t bytes = o_.limit
        ? std::min(static_cast<std::size_t>(avail), *o_.limit)
        : static_cast<std::size_t>(avail);
    return (bytes + o_.cols - 1) / o_.cols;
}

std::size_t Viewer::vis_rows() const {
    // 5 fixed UI lines: title + col-header + top-sep + bot-sep + status
    int n = tr_ - 5;
    return n > 1 ? static_cast<std::size_t>(n) : 1;
}

std::size_t Viewer::max_top() const {
    std::size_t tot = total_rows();
    std::size_t vis = vis_rows();
    return tot > vis ? tot - vis : 0;
}

// ── File I/O ──────────────────────────────────────────────────────────────────

std::vector<uint8_t> Viewer::read_row(std::size_t r) {
    std::size_t off  = o_.skip + r * o_.cols;
    std::size_t want = o_.cols;
    if (o_.limit) {
        std::size_t end = o_.skip + *o_.limit;
        if (off >= end) return {};
        want = std::min(want, end - off);
    }
    if (std::fseek(fp_, static_cast<long>(off), SEEK_SET) != 0) return {};
    std::vector<uint8_t> b(want);
    std::size_t n = std::fread(b.data(), 1, want, fp_);
    b.resize(n);
    return b;
}

// ── Drawing ───────────────────────────────────────────────────────────────────

void Viewer::draw_bar(int y, const std::string& text) {
    auto s = text;
    int pad = tc_ - static_cast<int>(text.size());
    if (pad > 0) s.append(static_cast<std::size_t>(pad), ' ');
    else if (pad < 0) s.resize(static_cast<std::size_t>(tc_));
    term::goto_xy(y, 0);
    std::cout << CC::BAR << s << CC::RST;
    term::clear_to_eol();
}

void Viewer::draw_sep(int y) {
    auto w = std::min(static_cast<std::size_t>(tc_), row_w(o_.cols, o_.ascii));
    term::goto_xy(y, 0);
    std::cout << CC::DIM << std::string(w, '-') << CC::RST;
    term::clear_to_eol();
}

void Viewer::draw_header(int y) {
    term::goto_xy(y, 0);
    print_header(o_.cols, o_.ascii, o_.upper);
    term::clear_to_eol();
}

void Viewer::draw_data_row(int y, std::size_t off, const uint8_t* data, std::size_t n) {
    term::goto_xy(y, 0);
    print_row(off, data, n, o_.cols, o_.ascii, o_.upper);
    term::clear_to_eol();
}

void Viewer::redraw() {
    term::get_size(tr_, tc_);
    term::cursor_home();   // jump to top without clearing — avoids flicker

    // Row 0 — title bar
    draw_bar(0, sfmt(" MyHex  %s  (%lld bytes) ", o_.path.c_str(), sz_));

    // Row 1 — column-index header
    draw_header(1);

    // Row 2 — top separator
    draw_sep(2);

    // Rows 3 .. tr_-3 — hex data
    const std::size_t vis = vis_rows();
    const std::size_t tot = total_rows();
    for (std::size_t i = 0; i < vis; ++i) {
        int y = static_cast<int>(3 + i);
        std::size_t r = top_ + i;
        if (r < tot) {
            auto d = read_row(r);
            draw_data_row(y, o_.skip + r * o_.cols, d.data(), d.size());
        } else {
            term::goto_xy(y, 0);
            term::clear_to_eol();
        }
    }

    // Row tr_-2 — bottom separator
    draw_sep(tr_ - 2);

    // Row tr_-1 — status bar
    const std::size_t last = std::min(top_ + vis, tot);
    draw_bar(tr_ - 1,
             sfmt(" Up/Down:Row  PgUp/PgDn:Page  Home/End  g:Goto  q:Quit"
                  "    Row %zu-%zu / %zu ",
                  top_ + 1, last, tot));

    term::clear_below();                      // clear leftover lines on resize
    term::goto_xy(tr_ - 1, tc_ - 1);         // park cursor in bottom-right corner
    std::cout.flush();
}

// ── Goto dialog ───────────────────────────────────────────────────────────────

void Viewer::goto_dlg() {
    const std::string prompt = " Goto offset (hex): 0x";
    const int prompt_col = static_cast<int>(prompt.size());

    draw_bar(tr_ - 1,
             prompt + std::string(
                 static_cast<std::size_t>(std::max(0, tc_ - prompt_col)), ' '));
    term::goto_xy(tr_ - 1, prompt_col);
    term::cursor_show();
    std::cout.flush();

    std::string input;
    for (;;) {
        int ch = term::read_key();
        if (ch == term::K_ENTER) break;
        if (ch == term::K_ESC)   { input.clear(); break; }
        if (ch == term::K_BS) {
            if (!input.empty()) {
                input.pop_back();
                int col = prompt_col + static_cast<int>(input.size());
                term::goto_xy(tr_ - 1, col);
                std::cout << ' ';
                term::goto_xy(tr_ - 1, col);
                std::cout.flush();
            }
            continue;
        }
        if (std::isxdigit(ch) && input.size() < 16) {
            char uc = static_cast<char>(std::toupper(ch));
            input += uc;
            std::cout << uc;
            std::cout.flush();
        }
    }

    term::cursor_hide();

    if (!input.empty()) {
        try {
            std::size_t target = std::stoul(input, nullptr, 16);
            if (target >= o_.skip) {
                std::size_t row = (target - o_.skip) / o_.cols;
                std::size_t vis = vis_rows();
                // Centre the target row on screen
                top_ = row > vis / 2 ? row - vis / 2 : 0;
                top_ = std::min(top_, max_top());
            }
        } catch (...) {}
    }
    redraw();
}

// ── Event loop ────────────────────────────────────────────────────────────────

void Viewer::run() {
    term::init();
    term::cursor_hide();
    term::clear_screen();
    std::cout.flush();

    redraw();

    bool alive = true;
    while (alive) {
        int ch = term::read_key();

        const std::size_t vis = vis_rows();
        const std::size_t mt  = max_top();
        bool changed = true;

        switch (ch) {
            case 'q': case 'Q':
            case term::K_ESC:    alive = false; changed = false; break;
            case 'g': case 'G':  goto_dlg();   changed = false; break;
            case term::K_UP:     if (top_ > 0) --top_; else changed = false; break;
            case term::K_DOWN:   if (top_ < mt) ++top_; else changed = false; break;
            case term::K_PGUP:   top_ = top_ >= vis ? top_ - vis : 0; break;
            case term::K_PGDN:   top_ = std::min(top_ + vis, mt); break;
            case term::K_HOME:   top_ = 0;  break;
            case term::K_END:    top_ = mt; break;
            case term::K_RESIZE: break;   // SIGWINCH (POSIX) — just redraw
            default:             changed = false; break;
        }
        if (changed) redraw();
    }

    term::cursor_show();
    term::clear_screen();
    if (fp_) { std::fclose(fp_); fp_ = nullptr; }
    term::restore();
    std::cout.flush();
}
