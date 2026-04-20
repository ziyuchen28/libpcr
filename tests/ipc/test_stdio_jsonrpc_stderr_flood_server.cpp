#include "pcr/framing/any_framer.h"
#include "pcr/framing/content_length_framer.h"
#include "pcr/jsonrpc/dispatcher.h"
#include "pcr/stream/any_stream.h"
#include "pcr/stream/pipe_stream.h"

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <unistd.h>

namespace {

void write_all(int fd, const char* data, std::size_t size)
{
    std::size_t off = 0;
    while (off < size) {
        const ssize_t n = ::write(fd, data + off, size - off);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            throw std::runtime_error(
                std::string("write failed: ") + std::strerror(errno));
        }
        off += static_cast<std::size_t>(n);
    }
}

void flood_stderr()
{
    std::string chunk(4096, 'E');
    chunk.back() = '\n';

    // If stderr is an unread pipe, this child should block here.
    for (int i = 0; i < 1024; ++i) {
        write_all(STDERR_FILENO, chunk.data(), chunk.size());
    }
}

} // namespace

int main()
{
    using namespace pcr;

    flood_stderr();

    stream::AnyStream io{stream::PipeDuplex(
        STDIN_FILENO,
        STDOUT_FILENO,
        stream::FdOwnership::Borrowed,
        stream::FdOwnership::Borrowed
    )};

    jsonrpc::Dispatcher rpc(
        framing::AnyFramer{
            framing::ContentLengthFramer(io)
        }
    );

    rpc.on_request("ping", [](const jsonrpc::Request&) {
        return jsonrpc::HandlerResult::ok(R"({"pong":true})");
    });

    while (rpc.pump_once()) {
    }

    return 0;
}
