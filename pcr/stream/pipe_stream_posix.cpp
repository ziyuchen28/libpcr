#include "pcr/stream/pipe_stream.h"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <string_view>

#include <unistd.h>

namespace pcr::stream {

[[noreturn]] static void throw_errno(std::string_view prefix) 
{
    throw std::runtime_error(std::string(prefix) + ": " + std::strerror(errno));
}

static void close_fd_if_open(int &fd) noexcept 
{
    if (fd >= 0) {
        ::close(fd);
        fd = -1;
    }
}


/* reader */
PipeReader::PipeReader(int read_fd, FdOwnership ownership)
    : fd_(read_fd),
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


std::size_t PipeReader::read_some(void *dst, std::size_t max_bytes) 
{
    if (!open_ || fd_ < 0) {
        throw std::logic_error("PipeReader: read_some on closed reader");
    }
    if (max_bytes == 0) {
        return 0;
    }
    for (;;) {
        const ssize_t n = ::read(fd_, dst, max_bytes);
        if (n < 0) {
            if (errno == EINTR) continue;
            throw_errno("read failed");
        }
        return static_cast<std::size_t>(n); // 0 => EOF
    }
}


void PipeReader::close_read() noexcept 
{
    if (!open_) return;
    open_ = false;

    if (!owned_) return;
    close_fd_if_open(fd_);
}


/* writer */

PipeWriter::PipeWriter(int write_fd, FdOwnership ownership)
    : fd_(write_fd),
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


PipeWriter &PipeWriter::operator=(PipeWriter &&other) noexcept 
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
    if (!open_ || fd_ < 0) {
        throw std::logic_error("PipeWriter: write_some on closed writer");
    }

    if (max_bytes == 0) {
        return 0;
    }

    for (;;) {
        const ssize_t n = ::write(fd_, src, max_bytes);
        if (n < 0) {
            if (errno == EINTR) continue;
            // NOTE: EPIPE may also raise SIGPIPE depending on process signal disposition.
            throw_errno("write failed");
        }
        return static_cast<std::size_t>(n);
    }
}

// to do
void PipeWriter::close_write() noexcept 
{
    if (!open_) return;
    open_ = false;
    if (!owned_) return;
    close_fd_if_open(fd_);
}


/* duplex */
PipeDuplex::PipeDuplex(
    int read_fd,
    int write_fd,
    FdOwnership read_ownership,
    FdOwnership write_ownership)
    : reader_(read_fd, read_ownership),
      writer_(write_fd, write_ownership) {}

PipeDuplex::PipeDuplex(PipeReader reader, PipeWriter writer) noexcept
    : reader_(std::move(reader)),
      writer_(std::move(writer)) 
{}

std::size_t PipeDuplex::read_some(void *dst, std::size_t max_bytes) 
{
    return reader_.read_some(dst, max_bytes);
}


std::size_t PipeDuplex::write_some(const void *src, std::size_t max_bytes) 
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



