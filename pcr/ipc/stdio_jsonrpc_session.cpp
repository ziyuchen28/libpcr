#include <pcr/ipc/stdio_jsonrpc_session.h>

#include <stdexcept>
#include <utility>

#include <pcr/channel/any_stream.h>
#include <pcr/channel/pipe_stream.h>
#include <pcr/framing/any_framer.h>
#include <pcr/framing/content_length_framer.h>
#include <pcr/rpc/any_codec.h>
#include <pcr/rpc/codec/nlohmann.h>
#include <pcr/rpc/dispatcher.h>
#include <pcr/rpc/peer.h>

namespace pcr::ipc {

struct StdioJsonRpcSession::Impl {
    pcr::proc::PipedChild child;
    pcr::channel::AnyStream io;
    pcr::rpc::Dispatcher rpc;

    explicit Impl(pcr::proc::PipedChild c)
        : child(std::move(c)),
          io(pcr::channel::PipeDuplex(
              child.stdout_read_fd(),
              child.stdin_write_fd(),
              pcr::channel::FdOwnership::Borrowed,
              pcr::channel::FdOwnership::Borrowed)),
          rpc(pcr::rpc::Peer(
              pcr::framing::AnyFramer{
                  pcr::framing::ContentLengthFramer(io)},
              pcr::rpc::AnyCodec{
                  pcr::rpc::NlohmannCodec{}})) {}
};

StdioJsonRpcSession::StdioJsonRpcSession(pcr::proc::PipedChild child)
    : impl_(std::make_unique<Impl>(std::move(child))) {}

StdioJsonRpcSession StdioJsonRpcSession::spawn(const pcr::proc::ProcessSpec& spec) {
    return StdioJsonRpcSession(pcr::proc::PipedChild::spawn(spec));
}

StdioJsonRpcSession::StdioJsonRpcSession(StdioJsonRpcSession&&) noexcept = default;
StdioJsonRpcSession& StdioJsonRpcSession::operator=(StdioJsonRpcSession&&) noexcept = default;
StdioJsonRpcSession::~StdioJsonRpcSession() = default;

pcr::proc::PipedChild& StdioJsonRpcSession::child() noexcept {
    return impl_->child;
}

const pcr::proc::PipedChild& StdioJsonRpcSession::child() const noexcept {
    return impl_->child;
}

pcr::rpc::Id StdioJsonRpcSession::send_request(
        std::string method,
        std::optional<std::string> params_json) {
    return impl_->rpc.send_request(std::move(method), std::move(params_json));
}

void StdioJsonRpcSession::send_notification(
        std::string method,
        std::optional<std::string> params_json) {
    impl_->rpc.send_notification(std::move(method), std::move(params_json));
}

void StdioJsonRpcSession::on_request(std::string method, RequestHandler handler) {
    impl_->rpc.on_request(
        std::move(method),
        [handler = std::move(handler)](const pcr::rpc::Request& req) -> pcr::rpc::HandlerResult {
            HandlerResult out = handler(req);
            if (out.error.has_value()) {
                return pcr::rpc::HandlerResult::fail(std::move(*out.error));
            }
            return pcr::rpc::HandlerResult::ok(
                out.result_json ? std::move(*out.result_json) : std::string("null"));
        });
}

void StdioJsonRpcSession::on_notification(
        std::string method,
        NotificationHandler handler) {
    impl_->rpc.on_notification(std::move(method), std::move(handler));
}

bool StdioJsonRpcSession::pump_once() {
    return impl_->rpc.pump_once();
}

std::optional<pcr::rpc::Response> StdioJsonRpcSession::take_response(
        const pcr::rpc::Id& id) {
    return impl_->rpc.take_response(id);
}

pcr::rpc::Response StdioJsonRpcSession::request(
        std::string method,
        std::optional<std::string> params_json) {
    const pcr::rpc::Id id = send_request(std::move(method), std::move(params_json));

    for (;;) {
        if (auto response = take_response(id); response.has_value()) {
            return std::move(*response);
        }

        if (!pump_once()) {
            throw std::runtime_error("JSON-RPC EOF while waiting for response");
        }
    }
}

std::string StdioJsonRpcSession::request_json(
        std::string method,
        std::optional<std::string> params_json,
        const char* error_prefix) {
    pcr::rpc::Response response = request(std::move(method), std::move(params_json));

    if (response.error.has_value()) {
        throw std::runtime_error(
            std::string(error_prefix) + ": " + response.error->message);
    }

    return response.result_json ? *response.result_json : "null";
}

void StdioJsonRpcSession::close_stdin() {
    impl_->child.close_stdin_write();
}

pcr::proc::WaitResult StdioJsonRpcSession::wait() {
    return impl_->child.wait();
}

} // namespace pcr::ipc
