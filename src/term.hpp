#pragma once

/// Cross-platform terminal abstraction.
///
/// Windows : _getch() + Console API  + ANSI escape codes
/// POSIX   : termios raw mode + select() + ANSI escape codes
///
/// No ncurses dependency — ANSI codes work on Windows 10+,
/// Linux, and macOS out of the box.
namespace term {

/// Logical key codes above the ASCII range (≥ 256) to avoid collision.
enum Key : int {
    K_UP     = 256,
    K_DOWN   = 257,
    K_PGUP   = 258,
    K_PGDN   = 259,
    K_HOME   = 260,
    K_END    = 261,
    K_RESIZE = 262,   ///< SIGWINCH / terminal resize (POSIX only)
    K_ESC    = 27,
    K_ENTER  = 13,
    K_BS     = 127,
    K_NONE   = -1,
};

/// Enter raw / interactive terminal mode.
void init();

/// Restore the terminal to its state before init().
void restore();

/// Query current terminal dimensions.
void get_size(int& rows, int& cols);

/// Block until a key is available, then return its logical code.
int read_key();

// ── ANSI output helpers (shared across all platforms) ────────────────────────

void cursor_hide();
void cursor_show();
void clear_screen();
void cursor_home();
void clear_to_eol();
void clear_below();

/// Move cursor to (row, col) — both 0-based.
void goto_xy(int row, int col);

} // namespace term
