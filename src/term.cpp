#include "term.hpp"
#include <iostream>

// ─────────────────────────── Windows ─────────────────────────────────────────
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <conio.h>

namespace term {

static DWORD  s_orig_mode = 0;
static HANDLE s_hout      = INVALID_HANDLE_VALUE;

void init() {
    s_hout = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode(s_hout, &s_orig_mode);
    // 0x0004 = ENABLE_VIRTUAL_TERMINAL_PROCESSING (ANSI escape codes)
    // 0x0008 = DISABLE_NEWLINE_AUTO_RETURN
    SetConsoleMode(s_hout, s_orig_mode | 0x0004 | 0x0008);
}

void restore() {
    if (s_hout != INVALID_HANDLE_VALUE)
        SetConsoleMode(s_hout, s_orig_mode);
}

void get_size(int& rows, int& cols) {
    CONSOLE_SCREEN_BUFFER_INFO i;
    if (GetConsoleScreenBufferInfo(s_hout, &i)) {
        rows = i.srWindow.Bottom - i.srWindow.Top + 1;
        cols = i.srWindow.Right  - i.srWindow.Left + 1;
    } else {
        rows = 24; cols = 80;
    }
}

int read_key() {
    int ch = _getch();
    if (ch == 0 || ch == 0xE0) {   // special key prefix
        switch (_getch()) {
            case 72: return K_UP;
            case 80: return K_DOWN;
            case 73: return K_PGUP;
            case 81: return K_PGDN;
            case 71: return K_HOME;
            case 79: return K_END;
            default: return K_NONE;
        }
    }
    if (ch == 27)   return K_ESC;
    if (ch == 13)   return K_ENTER;
    if (ch == '\b') return K_BS;
    return ch;
}

// ─────────────────────────── POSIX ───────────────────────────────────────────
#else

#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>
#include <csignal>
#include <cerrno>

namespace term {

static struct termios        s_orig;
static volatile sig_atomic_t s_resize = 0;

static void on_sigwinch(int) { s_resize = 1; }

void init() {
    tcgetattr(STDIN_FILENO, &s_orig);

    struct termios raw = s_orig;
    raw.c_iflag &= ~(unsigned)(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_cflag |=  CS8;
    raw.c_lflag &= ~(unsigned)(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    struct sigaction sa{};
    sa.sa_handler = on_sigwinch;
    sa.sa_flags   = 0;   // no SA_RESTART — lets read() return EINTR on resize
    sigaction(SIGWINCH, &sa, nullptr);
}

void restore() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &s_orig);
}

void get_size(int& rows, int& cols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) {
        rows = ws.ws_row;
        cols = ws.ws_col;
    } else {
        rows = 24; cols = 80;
    }
}

/// Read one byte with a timeout in milliseconds.
/// Returns 1 on success, 0 on timeout, -1 on error.
static int read_byte_tmo(unsigned char* c, int ms) {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    struct timeval tv = { ms / 1000, (ms % 1000) * 1000 };
    int r = select(STDIN_FILENO + 1, &fds, nullptr, nullptr, &tv);
    if (r <= 0) return r;
    return static_cast<int>(read(STDIN_FILENO, c, 1));
}

int read_key() {
    unsigned char ch;
    for (;;) {
        ssize_t n = read(STDIN_FILENO, &ch, 1);
        if (n == 1) break;
        if (n < 0 && errno == EINTR) {
            if (s_resize) { s_resize = 0; return K_RESIZE; }
            continue;
        }
        return K_NONE;
    }

    if (ch != 27) {
        if (ch == 127 || ch == '\b') return K_BS;
        if (ch == '\r' || ch == '\n') return K_ENTER;
        return static_cast<int>(ch);
    }

    // ESC — peek ahead for escape sequence (50 ms)
    unsigned char b;
    if (read_byte_tmo(&b, 50) <= 0) return K_ESC;
    if (b != '[') return K_ESC;
    if (read_byte_tmo(&b, 50) <= 0) return K_ESC;

    switch (b) {
        case 'A': return K_UP;
        case 'B': return K_DOWN;
        case 'H': return K_HOME;
        case 'F': return K_END;
    }

    // Extended sequences: ESC [ N ~
    if (b >= '1' && b <= '9') {
        unsigned char tilde;
        if (read_byte_tmo(&tilde, 50) <= 0 || tilde != '~') return K_ESC;
        switch (b) {
            case '1': return K_HOME;
            case '4': return K_END;
            case '5': return K_PGUP;
            case '6': return K_PGDN;
        }
    }
    return K_ESC;
}

#endif  // POSIX

// ── Shared ANSI output helpers ────────────────────────────────────────────────

void cursor_hide()         { std::cout << "\033[?25l"; }
void cursor_show()         { std::cout << "\033[?25h"; }
void clear_screen()        { std::cout << "\033[2J\033[H"; }
void cursor_home()         { std::cout << "\033[H"; }
void clear_to_eol()        { std::cout << "\033[K"; }
void clear_below()         { std::cout << "\033[J"; }
void goto_xy(int r, int c) { std::cout << "\033[" << r+1 << ";" << c+1 << "H"; }

} // namespace term
