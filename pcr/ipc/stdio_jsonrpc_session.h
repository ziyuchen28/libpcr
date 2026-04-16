#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include <piped_child.h>
#include <proc_spec.h>
#include <message.h>

namespace pcr::ipc {

struct HandlerResult {
    std::optional<std::string> result_json;
    std::optional<pcr::rpc::Error> error;

    static HandlerResult ok(std::string result_json_) {
        HandlerResult out;
        out.result_json = std::move(result_json_);
        return out;
    }

    static HandlerResult fail(pcr::rpc::Error error_) {
        HandlerResult out;
        out.error = std::move(error_);
        return out;
    }
};

using RequestHandler =
    std::function<HandlerResult(const pcr::rpc::Request&)>;

using NotificationHandler =
    std::function<void(const pcr::rpc::Notification&)>;

// Concrete high-level facade for the common case:
//   child process stdio <-> Content-Length framing <-> JSON-RPC
class StdioJsonRpcSession {
public:
    StdioJsonRpcSession(pcr::proc::PipedChild child);

    static StdioJsonRpcSession spawn(const pcr::proc::ProcessSpec& spec);

    StdioJsonRpcSession(StdioJsonRpcSession&&) noexcept;
    StdioJsonRpcSession& operator=(StdioJsonRpcSession&&) noexcept;
    ~StdioJsonRpcSession();

    StdioJsonRpcSession(const StdioJsonRpcSession&) = delete;
    StdioJsonRpcSession& operator=(const StdioJsonRpcSession&) = delete;

    pcr::proc::PipedChild& child() noexcept;
    const pcr::proc::PipedChild& child() const noexcept;

    pcr::rpc::Id send_request(std::string method,
                              std::optional<std::string> params_json = std::nullopt);

    void send_notification(std::string method,
                           std::optional<std::string> params_json = std::nullopt);

    void on_request(std::string method, RequestHandler handler);
    void on_notification(std::string method, NotificationHandler handler);

    bool pump_once();
    std::optional<pcr::rpc::Response> take_response(const pcr::rpc::Id& id);

    // Blocking convenience: sends request, pumps until matching response arrives.
    pcr::rpc::Response request(std::string method,
                               std::optional<std::string> params_json = std::nullopt);

    // Blocking convenience: returns raw result JSON text.
    // Throws on EOF or JSON-RPC error response.
    std::string request_json(std::string method,
                             std::optional<std::string> params_json = std::nullopt,
                             const char* error_prefix = "request failed");

    void close_stdin();
    pcr::proc::WaitResult wait();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace pcr::ipc
