#include "pcr/stream/pipe_stream.h"

#include <cerrno>
#include <climits>
#include <cstring>
#include <stdexcept>
#include <string_view>

#include <io.h>
#include <fcntl.h>

namespace pcr::stream {


namespace {


[[noreturn]] void throw_errno(std::string_view prefix)
{
    throw std::runtime_error(std::string(prefix) + ": " + std::strerror(errno));
}

void close_fd_if_open(int &fd) noexcept
{
    if (fd >= 0) {
        ::_close(fd);
        fd = -1;
    }
}

unsigned int checked_io_size(std::size_t n)
{
    if (n > static_cast<std::size_t>(UINT_MAX)) {
        return UINT_MAX;
    }
    return static_cast<unsigned int>(n);
}

} // namespace


PipeReader::PipeReader(int fd, FdOwnership ownership)
    : fd_(fd), 
      owned_(ownership == FdOwnership::Owned)
{
    if (fd_ < 0) {
        throw std::invalid_argument("PipeReader requires non-negative fd");
    }
}

PipeReader::~PipeReader() noexcept
{
    close_read();
}


PipeReader::PipeReader(PipeReader &&other) noexcept
    : fd_(other.fd_),
      owned_(other.owned_),
      open_(other.open_) 
{
    other.fd_ = -1;
    other.owned_ = false;
    other.open_ = false;
}

PipeReader &PipeReader::operator=(PipeReader &&other) noexcept
{
    if (this == &other) return *this;

    close_read();

    fd_ = other.fd_;
    owned_ = other.owned_;
    open_ = other.open_;

    other.fd_ = -1;
    other.owned_ = false;
    other.open_ = false;

    return *this;
}

std::size_t PipeReader::read_some(void* dst, std::size_t max_bytes)
{
    if (fd_ < 0) {
        throw std::logic_error("PipeReader: read on closed fd");
    }
    if (max_bytes == 0) {
        return 0;
    }

    const unsigned int want = checked_io_size(max_bytes);
    const int n = ::_read(fd_, dst, want);
    if (n < 0) {
        throw_errno("_read failed");
    }

    return static_cast<std::size_t>(n);
}

void PipeReader::close_read() noexcept
{
    if (!open_) return;
    open_ = false;

    if (!owned_) return;
    close_fd_if_open(fd_);
}

PipeWriter::PipeWriter(int fd, FdOwnership ownership)
    : fd_(fd),
      owned_(ownership == FdOwnership::Owned) 
{
    if (fd_ < 0) {
        throw std::invalid_argument("PipeWriter requires non-negative fd");
    }
}

PipeWriter::~PipeWriter() noexcept
{
    close_write();
}

PipeWriter::PipeWriter(PipeWriter &&other) noexcept
    : fd_(other.fd_),
      owned_(other.owned_),
      open_(other.open_) 
{
    other.fd_ = -1;
    other.owned_ = false;
    other.open_ = false;
}


PipeWriter& PipeWriter::operator=(PipeWriter &&other) noexcept
{
    if (this == &other) return *this;

    close_write();

    fd_ = other.fd_;
    owned_ = other.owned_;
    open_ = other.open_;

    other.fd_ = -1;
    other.owned_ = false;
    other.open_ = false;

    return *this;
}


std::size_t PipeWriter::write_some(const void *src, std::size_t max_bytes)
{
    if (fd_ < 0) {
        throw std::logic_error("PipeWriter: write on closed fd");
    }
    if (max_bytes == 0) {
        return 0;
    }

    const unsigned int want = checked_io_size(max_bytes);
    const int n = ::_write(fd_, src, want);
    if (n < 0) {
        throw_errno("_write failed");
    }

    return static_cast<std::size_t>(n);
}


void PipeWriter::close_write() noexcept
{
    if (!open_) return;
    open_ = false;
    if (!owned_) return;
    close_fd_if_open(fd_);
}


PipeDuplex::PipeDuplex(
    int read_fd,
    int write_fd,
    FdOwnership read_ownership,
    FdOwnership write_ownership)
    : reader_(read_fd, read_ownership),
      writer_(write_fd, write_ownership) {}

PipeDuplex::PipeDuplex(PipeReader reader, PipeWriter writer) noexcept
    : reader_(std::move(reader)),
      writer_(std::move(writer)) {}

std::size_t PipeDuplex::read_some(void* dst, std::size_t max_bytes)
{
    return reader_.read_some(dst, max_bytes);
}

std::size_t PipeDuplex::write_some(const void* src, std::size_t max_bytes)
{
    return writer_.write_some(src, max_bytes);
}

void PipeDuplex::close_read()
{
    reader_.close_read();
}

void PipeDuplex::close_write()
{
    writer_.close_write();
}


} // namespace pcr::stream


