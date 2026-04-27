#pragma once

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <string_view>

#ifdef _WIN32
#include <io.h>
using ssize_t = intptr_t;
#define READ ::_read
#define WRITE ::_write
#else
#include <unistd.h>
#define READ ::read
#define WRITE ::write
#endif


inline void write_all_fd(int fd, std::string_view data) 
{
    std::size_t off = 0;
    while (off < data.size()) {
        const ssize_t n = WRITE(fd, data.data() + off, data.size() - off);
        if (n < 0) {
            if (errno == EINTR) continue;
            throw std::runtime_error(std::string("write failed: ") + std::strerror(errno));
        }
        off += static_cast<std::size_t>(n);
    }
}


inline std::string read_all_fd(int fd) 
{
    std::string out;
    char buf[4096];

    for (;;) {
        const ssize_t n = READ(fd, buf, sizeof(buf));
        if (n == 0) break;
        if (n < 0) {
            if (errno == EINTR) continue;
            throw std::runtime_error(std::string("read failed: ") + std::strerror(errno));
        }
        out.append(buf, static_cast<std::size_t>(n));
    }

    return out;
}


inline std::string trim_trailing_newlines(std::string s) 
{
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) {
        s.pop_back();
    }
    return s;
}
