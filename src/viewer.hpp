#pragma once
#include "opts.hpp"
#include <cstdio>
#include <vector>
#include <cstdint>
#include <cstddef>

/// Interactive TUI hex viewer.
///
/// Reads lazily from a binary file — only the visible rows are seeked
/// and read on each redraw. Handles keyboard navigation and terminal resize.
class Viewer {
public:
    explicit Viewer(const Opts& o);
    ~Viewer() = default;

    Viewer(const Viewer&)            = delete;
    Viewer& operator=(const Viewer&) = delete;

    /// Enter raw terminal mode and run the interactive event loop.
    /// Returns when the user presses q or Esc.
    void run();

private:
    std::FILE*    fp_;  ///< binary file handle (FILE* avoids GCC 8.x ifstream bug)
    long long     sz_;
    Opts          o_;
    std::size_t   top_ = 0;   ///< index of the topmost visible row
    int           tr_  = 24;  ///< terminal height (rows)
    int           tc_  = 80;  ///< terminal width  (cols)

    // ── Geometry ──────────────────────────────────────────────────────────────
    std::size_t total_rows() const;
    std::size_t vis_rows()   const;
    std::size_t max_top()    const;

    // ── File I/O ──────────────────────────────────────────────────────────────
    std::vector<uint8_t> read_row(std::size_t r);

    // ── Drawing ───────────────────────────────────────────────────────────────
    void draw_bar     (int y, const std::string& text);
    void draw_sep     (int y);
    void draw_header  (int y);
    void draw_data_row(int y, std::size_t off, const uint8_t* data, std::size_t n);
    void redraw();

    // ── Dialogs ───────────────────────────────────────────────────────────────
    void goto_dlg();
};
