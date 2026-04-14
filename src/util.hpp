#pragma once
#include <string>
#include <cstdio>

/// Type-safe snprintf wrapper — returns a std::string.
template<typename... A>
std::string sfmt(const char* fmt, A... args) {
    int n = std::snprintf(nullptr, 0, fmt, args...);
    if (n <= 0) return {};
    std::string s(static_cast<std::size_t>(n + 1), '\0');
    std::snprintf(s.data(), s.size(), fmt, args...);
    s.resize(static_cast<std::size_t>(n));
    return s;
}
